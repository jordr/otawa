/*
 *	$Id$
 *	
 *	This file is part of OTAWA
 *	Copyright (c) 2006-11, IRIT UPS.
 *	
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *	
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *	
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <elm/genstruct/HashTable.h>
#include <otawa/prog/sem.h>
#include <otawa/prog/Process.h>
#include <otawa/hard/Register.h>
#include <otawa/proc/CFGProcessor.h>
#include <otawa/util/LoopInfoBuilder.h>
#include <otawa/cfg/Virtualizer.h>
#include <otawa/util/HalfAbsInt.h>
#include <otawa/util/WideningListener.h>
#include <otawa/util/WideningFixPoint.h>
#include <otawa/data/clp/ClpValue.h>
#include <otawa/data/clp/ClpState.h>
#include <otawa/data/clp/ClpAnalysis.h>
#include <otawa/data/clp/ClpPack.h>
#include <otawa/data/clp/DeadCodeAnalysis.h>
#include <otawa/data/clp/SymbolicExpr.h>
#include <otawa/util/FlowFactLoader.h>
#include <otawa/ipet/FlowFactLoader.h>
 
using namespace elm;
using namespace otawa;
using namespace otawa::util;

// Debug output for the state
#define TRACES(t)	//t
// Debug output for the domain
#define TRACED(t)	//t
// Debug output for the problem
#define TRACEP(t)	//t
// Debug output for Update function 
#define TRACEU(t)	//t
// Debug output for instructions in the update function
#define TRACEI(t)	//t
#define STATE_MULTILINE

// enable to load data from segments when load results with T
//#define DATA_LOADER

namespace otawa {

/**
 * This identifier is a configuration for the @ref ClpAnalysis processor.
 * It allows to provide initial values for the registers involved in the analysis.
 * The argument is a pair of register and its initial value as an address.
 * A null address express the fact that the register is initialized with the default
 * stack pointer address.
 */
Identifier<ClpAnalysis::init_t> ClpAnalysis::INITIAL(
		"otawa::ClpAnalysis::INITIAL",
		pair((const hard::Register *)0, Address::null));

namespace clp {

/**
 * Return the gcd of two long integers
 */
long gcd(long a, long b){
	if(a < 0)
		a = -a;
	if(b < 0)
		b = -b;
	
	if (a == 0)
		return b;
	else if (b == 0)
		return a;
	else
		return gcd(b, a % b);
}

/**
 * Return the lcm of two long integers
*/
long lcm(long a, long b){
	return abs(a * b) / gcd(a, b);
}

/**
 * Return the min with a signed comparison
*/
OCLP_intn_t min(OCLP_intn_t a, OCLP_intn_t b){
	if ( a < b)
		return a;
	else
		return b;
}
/**
 * Return the max with a signed comparison
*/
OCLP_intn_t max(OCLP_intn_t a, OCLP_intn_t b){
	if ( a > b)
		return a;
	else
		return b;
}
/**
 * Return the min with an unsigned comparison
*/
OCLP_uintn_t umin(OCLP_uintn_t a, OCLP_uintn_t b){
	if (a < b)
		return a;
	else
		return b;
}
/**
 * Return the max with an usigned comparison
*/
OCLP_uintn_t umax(OCLP_uintn_t a, OCLP_uintn_t b){
	if ( a > b)
		return a;
	else
		return b;
}

/* *** Value methods *** */

Value Value::operator+(const Value& val) const{
	Value v = *this;
	v.add(val);
	return v;
}

Value Value::operator-(const Value& val) const{
	Value v = *this;
	v.sub(val);
	return v;
}

/**
 * Add another set to the current one
 * @param val the value to add
 */
void Value::add(const Value& val){
	if (_kind == NONE && val._kind == NONE) 	/* NONE + NONE = NONE */
		set(NONE, 0, 0, 0);
	else if (_kind == ALL || val._kind == ALL) 	/* ALL + anything = ALL */
		set(ALL, 0, 1, OCLP_UMAXn);
	else if (_delta == 0 && val._delta == 0) 	/* two constants */
		set(_kind, _lower + val._lower, 0, 0);
	else {										/* other cases */
		OCLP_uintn_t g = gcd(_delta, val._delta);
		set(VAL, start() + val.start(), g,
		    _mtimes * (abs(_delta) / g) + val._mtimes * (abs(val._delta) / g));
	}
}

/**
 * Subtract another set to the current one
 * @param val the value to subtract
 */
void Value::sub(const Value& val) {
	if (_kind == NONE && val._kind == NONE)		/* NONE - NONE = NONE */
		set(NONE, 0, 0, 0);
	else if (_kind == ALL || val._kind == ALL)	/* ALL - anything = ALL */
		set(ALL, 0, 1, OCLP_UMAXn);
	else if (_delta == 0 && val._delta == 0)	/* two constants */
		set(_kind, _lower - val._lower, 0, 0);
	else {										/* other cases */
		OCLP_uintn_t g = gcd(_delta, val._delta);
		set(VAL, start() - val.stop(), g,
			_mtimes * (abs(_delta) / g) + val._mtimes * (abs(val._delta) / g));
	}
}

/**
 * Print a human representation of the CLP
 * @param out the stream to output the representation
 */
void Value::print(io::Output& out) const {
	if (_kind == ALL)
		out << 'T';
	else if (_kind == NONE)
		out << 'N';
	else if ((_delta == 0) && (_mtimes ==  0))
		out << "k(0x" << io::hex(_lower) << ')';
		//out << "k(" << _lower << ')';
	else
		out << "(0x" << io::hex(_lower) << ", 0x" << io::hex(_delta) << \
			", 0x" << io::hex(_mtimes) << ')';
		//out << '(' << _lower << ", " << _delta << ", " << _mtimes << ')';
}

/**
 * Left shift the current value.
 * @param val the value to shift the current one with. Must be a positive
 *				constant.
*/
void Value::shl(const Value& val) {
	if(!OCLP_IS_CST(val) || val._lower < 0){
		set(ALL, 0, 1, OCLP_UMAXn);
	} else if (_kind != NONE && _kind != ALL) {
		if (_delta == 0 && _mtimes == 0)
			set(VAL, _lower << val._lower, 0, 0);
		else
			set(VAL, _lower << val._lower, _delta << val._lower, _mtimes);
	}
}

/**
 * Right shift the current value.
 * @param val the value to shift the current one with. Must be a positive
 *				constant.
*/
void Value::shr(const Value& val) {
	if(!OCLP_IS_CST(val) || val._lower < 0){
		set(ALL, 0, 1, OCLP_UMAXn);
	} else
	if (_kind != NONE && _kind != ALL) {
		if (_delta == 0 && _mtimes == 0)
			set(VAL, _lower >> val._lower, 0, 0);
		else if (_delta % 2 == 0)
			set(VAL, _lower >> val._lower, _delta >> val._lower, _mtimes);
		else
			set(VAL, _lower >> val._lower, 1,
				(_delta * _mtimes) >> val._lower);
	}
}

/**
 * Join another set to the current one
 * @param val the value to be joined with
 */
void Value::join(const Value& val) {
	if ((*this) == val)							/* A U A = A (nothing to do) */
		return;
	else if (_kind == ALL || val._kind == ALL)  /* ALL U anything = ALL */
		set(ALL, 0, 1, OCLP_UMAXn);
	else if (_kind == NONE)						/* NONE U A = A */
		set(VAL, val._lower, val._delta, val._mtimes);
	else if (val._kind == NONE)					/* A U NONE = A (nothing to do) */
		return;
	else if (_delta == 0 && _mtimes == 0 && OCLP_IS_CST(val)) /* k1 U k2 */
		set(VAL, min(_lower, val._lower), abs(_lower - val._lower), 1);
	else {										/* other cases */
		OCLP_uintn_t g = gcd(gcd(abs(start() - val.start()), _delta), val._delta);
		OCLP_intn_t ls = min(start(), val.start());
		int64_t u1 = (int64_t)start() + ((int64_t)abs(_delta))*(int64_t)_mtimes;
		int64_t u2 = (int64_t)val.start() + ((int64_t)abs(val._delta))*(int64_t)val._mtimes;
		int64_t umax;
		if (u1 > u2)
			umax = u1;
		else
			umax = u2;
		set(VAL, ls, g, (umax - ls) / g);
	}
}

/**
 * Perform a widening to the infinite (to be filtred later)
 * @param val the value of the next iteration state
*/
void Value::widening(const Value& val) {
	if (_kind == NONE && val._kind == NONE) /* widen(NONE, NONE) = NONE */
		return;
	else if (_kind == ALL || val._kind == ALL) /* widen(ALL, *) = ALL */
		set(ALL, 0, 1, OCLP_UMAXn);
	else if (*this == val)					/* this == val -> do nothing */
		return;
	else {
		if (_delta != val._delta && _delta != - val._delta){
			// set to T
			set(ALL, 0, 1, OCLP_UMAXn);
			return;
		}
		
		// two constants: the delta is | k1 - k2 |
		if ((OCLP_IS_CST((*this))) && OCLP_IS_CST(val)){
			_delta = abs(_lower - val._lower);
		}
		
		if (val.start() < start()){
			// go to negatives
			if(val.stop() > stop()){
				// and also to the positives
				set(ALL, 0, 1, OCLP_UMAXn);
				return;
			}
			OCLP_intn_t absd = abs(_delta);
			set(_kind, stop(), -absd, OCLP_UMAXn / absd);
			return;
		}
		if (val.stop() > stop()){
			// go the positive
			OCLP_intn_t absd = abs(_delta);
			set(_kind, start(), absd, OCLP_UMAXn / absd);
		}
	}
}

/**
 * Perform a widening, knowing flow facts for the loop
 * @param val the value of the next iteration state
 * @param loopBound the maximum number of iteration of the loop
*/
void Value::ffwidening(const Value& val, int loopBound){
	if (_kind == NONE && val._kind == NONE) /* widen(NONE, NONE) = NONE */
		return;
	else if (_kind == ALL || val._kind == ALL) /* widen(ALL, *) = ALL */
		set(ALL, 0, 1, OCLP_UMAXn);
	else if (*this == val)					/* this == val -> do nothing */
		return;
	else if (OCLP_IS_CST((*this)) && OCLP_IS_CST(val)) {
		if (_lower < val._lower)
			/* widen((k1, 0, 0), (k2, 0, 0)) = (k1, k2 - k1, N) */
			set(VAL, _lower, val._lower - _lower, loopBound);
		else {
			/* widen((k1, 0, 0), (k2, 0, 0)) = (k1-N(k1-k2),k1-k2,N) */
			int step = _lower - val._lower;
			set(VAL, _lower - loopBound * step, step, loopBound);
		}
	}
	else if ((_delta == val._delta) &&		/* avoid division by 0 */
			 ((val._lower - _lower) % _delta == 0) &&
			 (_mtimes >= val._mtimes))
		return;				/* val == this + k => this */
	else
		/* other cases: T */
		set(ALL, 0, 1, OCLP_UMAXn);
}

/**
 * Intersection with the current value.
 * @param val the value to do the intersection with
 */
void Value::inter(const Value& val) {
	OCLP_intn_t l1 = _lower, l2 = val._lower;
	OCLP_intn_t sta1 = start(), sta2 = val.start();
	OCLP_intn_t sto1 = stop(), sto2 = val.stop();
	OCLP_intn_t d1 = _delta, d2 = val._delta;
	OCLP_uintn_t m1 = _mtimes, m2 = val._mtimes;
	int64_t u1, u2;
	if (_delta < 0)
		u1 = _lower;
	else
		u1 = (int64_t)_lower + (int64_t)_delta * (int64_t)_mtimes;
	if (val._delta < 0)
		u2 = _lower;
	else
		u2 = (int64_t)val._lower + (int64_t)val._delta * (int64_t)val._mtimes;
	
	// In this function, numbers are refs to doc/inter/clpv2-inter.pdf
	
	// 2. Special cases ========================
	
	// 2.1. A n A (T n T)
	if ((*this) == val){
		// do nothing
		return;
	}
	
	// 2.2. cst n cst
	if (OCLP_IS_CST((*this)) && OCLP_IS_CST(val)){
		if (l1 == l2)
			set(VAL, l1, 0, 0);
		else
			set(NONE, 0, 0, 0);
		return ;
	}
	// 2.3. cst n clp || clp n cst
	if (OCLP_IS_CST((*this))) {
		if ( ((sta1 >= sta2) && ((sta1 - sta2) % d2 == 0) && (u1 <= u2)) ||
		     (val == all))
			set(VAL, sta1, 0, 0);
		else
			set(NONE, 0, 0, 0);
		return ;
	}
	if (OCLP_IS_CST(val)){
		if ( ((sta2 >= sta1) && ((sta2 - sta1) % d1 == 0) && (u2 <= u1)) ||
		     (*this == all))
			set(VAL, sta2, 0, 0);
		else
			set(NONE, 0, 0, 0);
		return;
	}
	// 2.4. not overlapping intervals
	OCLP_uintn_t l2test = (l2 - l1) / d1, m2test = (l2 + d2 * m2 - l1) / d1;
	OCLP_uintn_t l1test = (l1 - l2) / d2, m1test = (l1 + d1 * m1 - l2) / d2;
	
	if (!(	( (0 <= l2test) && (l2test <= m1) ) ||
			( (0 <= m2test) && (m2test <= m1) ) ||
			( (0 <= l1test) && (l1test <= m2) ) ||
			( (0 <= m1test) && (m1test <= m2) ) )){
		
		set(NONE, 0, 0, 0);
		return;
	}
	// 2.5 intersection with a continue interval
	if (d1 == 1 || d1 == -1){
		OCLP_intn_t ls;
		OCLP_uintn_t ms;
		if (d2 > 0){
			ls = max(l2, (OCLP_intn_t)ceil((float)(sta1 - l2)/d2) * d2 + l2);
			ms = umin(
				(OCLP_uintn_t)(sto1 - ls) / d2,
				(OCLP_uintn_t)(sto2 - ls) / d2
			);
		}else{
			ASSERT(d2 < 0); // the d2==0 case is 2.2 or 2.3
			ls = min(l2, ((sto1 - l2) / d2) * d2 + l2);
			ms = min(
				m2 + (l2 - ls) / d2,
				(OCLP_uintn_t)(ls - sta1) / -d2
			);
		}
		// normalize constants
		if(ms == 0)
			d2 = 0;
		set(VAL, ls, d2, ms);
		return;
	}
	if (d2 == 1 || d2 == -1){
		OCLP_intn_t ls;
		OCLP_uintn_t ms;
		if (d1 > 0){
			ls = max(l1, (OCLP_intn_t)ceil((float)(sta2 - l1)/d1) * d1 + l1);
			ms = umin(
				(OCLP_uintn_t)(sto2 - ls) / d1,
				(OCLP_uintn_t)(sto1 - ls) / d1
			);
		}else{
			ASSERT(d1 < 0); // the d1==0 case is 2.2 or 2.3
			ls = min(l1, ((sto2 - l1) / d1) * d1 + l1);
			ms = min(
				m1 + (l1 - ls) / d1,
				(OCLP_uintn_t)(ls - sta2) / -d1
			);
		}
		// normalize constants
		if(ms == 0)
			d1 = 0;
		set(VAL, ls, d1, ms);
		return;
	}
	
	
	// 3. Main case ==========================
	OCLP_uintn_t d = gcd(d1, d2);
	
	// 3.1. Test if a solution exists
	if ((l2 - l1) % d != 0){
		set(NONE, 0, 0, 0);
		return;
	}
	
	// 3.2. ds: step of the solution
	OCLP_uintn_t ds = lcm(d1, d2);
	
	// 3.4. Research of a particular solution
	bool solution_found = false;
	long ip1p;
	for(OCLP_uintn_t i = 1; i < (OCLP_uintn_t)abs(d2); i++){
		if((d1 * i - 1) % d2 == 0){
			ip1p = i;
			solution_found = true;
			break;
		}
	}
	// FIXME: this case should not append (see 3.1)
	if(!solution_found){
		set(NONE, 0, 0, 0);
		return;
	}
	long i1p = ip1p * (l2 - l1);
	long i2p = (1 - d1 * ip1p) / (- d2) * (l2 - l1);
	
	// 3.5. min of the intersection (ls)
	long k = max(ceil(-i1p * (float)d1 / ds), ceil(-i2p * (float)d2 / ds));
	OCLP_uintn_t i1s = i1p + k * ds / d1;
	OCLP_uintn_t i2s = i2p + k * ds / d2;
	OCLP_intn_t ls = l1 + d1 * i1s;
	ASSERT((OCLP_uintn_t)ls == l2 + d2 * i2s);
	
	// 3.6. mtimes of the intersection (ms)
	OCLP_intn_t umin = min(l1 + d1 * m1, l2 + d2 * m2);
	OCLP_uintn_t ms = floor((float)(umin-ls) / ds);
	
	// normalize constants
	if((ds == 0) || (ms == 0)){
		ds = 0;
		ms = 0;
	}
	
	// set the result!
	set(VAL, ls, ds, ms);
}

inline io::Output& operator<<(io::Output& out, const Value& v) { v.print(out); return out; }
const Value Value::none(NONE), Value::all(ALL, 0, 1, OCLP_UMAXn); 

/* *** State methods *** */

/**
 * Change the state to be a copy of the given one
 * @param state the state to be copied
*/
void State::copy(const State& state) {
	TRACED(cerr << "copy("; print(cerr); cerr << ", "; state.print(cerr); cerr << ") = ");
	clear();
	// memory
	first = state.first;
	for(Node *prev = &first, *cur = state.first.next; cur; cur = cur->next) {
		prev->next = new Node(cur);
		prev = prev->next;
	}
	// registers
	registers.copy(state.registers);
	tmpreg.copy(state.tmpreg);
	
	TRACED(print(cerr); cerr << io::endl);
}
	
/**
 * Remove all nodes from the state
*/
void State::clear(void) {
	
	// registers
	registers.clear();
	tmpreg.clear();
	
	// memory
	for(Node *cur = first.next, *next; cur; cur = next) {
		next = cur->next;
		delete cur;
	}
	first.next = 0;
}

/**
 * Define a value into a register or the memory
 * @param addr a value of kind REG for a register, VAL for the memory.
 *		  The value must be a constant (only the lower() attribute will be
 *		  used) or all.
 * @param val the value to store at the given address
*/
void State::set(const Value& addr, const Value& val) {
	TRACED(cerr << "set("; print(cerr); cerr << ", " << addr << ", " << val << ") = ");
	Node *prev, *cur, *next;
	if(first.val == Value::none) {
		TRACED(print(cerr); cerr << io::endl);
		return;
	}
	
	// insert a none in the state (put the state to none)
	if (val == Value::none){
		clear();
		first.val = Value::none;
		return;
	}
	
	// we assume that addr is a constant... (or T)
	ASSERT(OCLP_IS_CST(addr) || addr == Value::all);
	
	// == Register ==
	if(addr.kind() == REG) {
		if (addr.lower() < 0){
			// temp ones
			if (-addr.lower() < tmpreg.length())
				tmpreg.set(-addr.lower(), val);
			else {
				for(int i = tmpreg.length(); i < -addr.lower(); i++)
					tmpreg.add(Value::all);
				tmpreg.add(val);
			}
		} else {
			// real ones
			if (addr.lower() < registers.length())
				registers.set(addr.lower(), val);
			else {
				for(int i = registers.length(); i < addr.lower(); i++)
					registers.add(Value::all);
				registers.add(val);
			}
		}
		return;
	}
		
	// == or Memory ==
	
	// consum all memory references
	if(addr == Value::all) {
		prev = &first;
		cur = first.next;
		while(cur) {
			next = cur->next;
			delete cur;
			cur = next;
		}
		prev->next = 0;
		return;
	}

	// find a value
	else {
		for(prev = &first, cur = first.next; cur && cur->addr < addr.lower(); prev = cur, cur = cur->next);
		if(cur && cur->addr == addr.lower()) {
			if(val.kind() != ALL)
				cur->val = val;
			else {
				prev->next = cur->next;
				delete cur;
			}
		}
		else if(val.kind() != ALL) {
			next = new Node(addr.lower(), val);
			prev->next = next;
			prev->next->next = cur;
		}
	}
	TRACED(print(cerr); cerr << io::endl);
}
	
/**
 * @return if a state is equals to the current one
*/
bool State::equals(const State& state) const {
	
	// Registers
	if (registers.length() != state.registers.length())
		return false;
	
	for (int i=0; i < registers.length(); i++)
		if (registers[i] != state.registers[i])
			return false;
	// Tmp registers
	/*for (int i=0; i < tmpreg.length(); i++)
		if (tmpreg[i] != state.tmpreg[i])
			return false;*/
	
	// Memory
	if(first.val.kind() != state.first.val.kind())
		return false;
	
	Node *cur = first.next, *cur2 = state.first.next;
	while(cur && cur2) {
		if(cur->addr != cur2->addr)
			return false;
		if(cur->val != cur->val)
			return false;
		cur = cur->next;
		cur2 = cur2->next;
	}
	return cur == cur2;
}

/**
 * Merge a state with the current one.
*/
void State::join(const State& state) {
	TRACED(cerr << "join(\n\t"; print(cerr); cerr << ",\n\t";  state.print(cerr); cerr << "\n\t) = ");
	
	// test none states
	if(state.first.val == Value::none)
		return;
	if(first.val == Value::none) {
		copy(state);
		TRACED(print(cerr); cerr << io::endl;);
		return;
	}
	
	// registers
	for(int i=0; i<registers.length() && i<state.registers.length() ; i++)
		registers[i].join(state.registers[i]);
	if (registers.length() < state.registers.length())
		for(int i=registers.length(); i < state.registers.length(); i++)
			registers.add(state.registers[i]);
	// temp registers
	for(int i=0; i<tmpreg.length() && i<state.tmpreg.length() ; i++)
		tmpreg[i].join(state.tmpreg[i]);
	if (tmpreg.length() < state.tmpreg.length())
		for(int i=tmpreg.length(); i < state.tmpreg.length(); i++)
			tmpreg.add(state.tmpreg[i]);
	
	// memory
	Node *prev = &first, *cur = first.next, *cur2 = state.first.next, *next;
	while(cur && cur2) {
		
		// addr1 < addr2 -> remove cur1
		if(cur->addr < cur2->addr) {
			prev->next = cur->next;
			delete cur;
			cur = prev->next;
		}
		
		// equality ? remove if join result in all
		else if(cur->addr == cur2->addr) {
			cur->val.join(cur2->val);
			if(cur->val.kind() == ALL) {
				prev->next = cur->next;
				delete cur;
				cur = prev->next;
			}
			else {
				prev = cur;
				cur = cur->next;
				cur2 = cur2->next;
			}
		}
		
		// addr1 > addr2 => remove cur2
		else
			cur2 = cur2->next;
	}
	
	// remove tail
	prev->next = 0;
	while(cur) {
		next = cur->next;
		delete cur;
		cur = next;
	}
	TRACED(print(cerr); cerr << io::endl;);
}

/**
 * Perform a widening.
 * @param state the state of the next iteration
 * @param loopBound is the number of iteration of the loop. A different widening
 *        operation will be used if the loopBound is known (>=0) or not.
*/
void State::widening(const State& state, int loopBound) {
	TRACED(cerr << "widening(" << loopBound << "\n\t");
	TRACED(print(cerr); cerr << ",\n\t";  state.print(cerr); cerr << "\n\t) = ");
	
	// test none states
	if(state.first.val == Value::none)
		return;
	if(first.val == Value::none) {
		copy(state);
		TRACED(print(cerr); cerr << io::endl;);
		return;
	}
	
	// registers
	for(int i=0; i<registers.length() && i<state.registers.length() ; i++)
		if (loopBound >= 0)
			registers[i].ffwidening(state.registers[i], loopBound);
		else
			registers[i].widening(state.registers[i]);
	if (registers.length() < state.registers.length())
		for(int i=registers.length(); i < state.registers.length(); i++)
			registers.add(state.registers[i]);
	// tmp registers
	for(int i=0; i<tmpreg.length() && i<state.tmpreg.length() ; i++)
		if (loopBound >= 0)
			tmpreg[i].ffwidening(state.tmpreg[i], loopBound);
		else
			tmpreg[i].widening(state.tmpreg[i]);
	if (tmpreg.length() < state.tmpreg.length())
		for(int i=tmpreg.length(); i < state.tmpreg.length(); i++)
			tmpreg.add(state.tmpreg[i]);
	
	// memory
	Node *prev = &first, *cur = first.next, *cur2 = state.first.next, *next;
	while(cur && cur2) {
		// addr1 < addr2 -> remove cur1
		if(cur->addr < cur2->addr) {
			prev->next = cur->next;
			delete cur;
			cur = prev->next;
		}
		// equality ? remove if join result in all
		else if(cur->addr == cur2->addr) {
			if (loopBound >= 0)
				cur->val.ffwidening(cur2->val, loopBound);
			else
				cur->val.widening(cur2->val);
			if(cur->val.kind() == ALL) {
				prev->next = cur->next;
				delete cur;
				cur = prev->next;
			}
			else {
				prev = cur;
				cur = cur->next;
				cur2 = cur2->next;
			}
		}
		// addr1 > addr2 => remove cur2
		else
			cur2 = cur2->next;
	}
	
	// remove tail
	prev->next = 0;
	while(cur) {
		next = cur->next;
		delete cur;
		cur = next;
	}
	TRACED(print(cerr); cerr << io::endl;);
}

/**
 * Print the state
*/
void State::print(io::Output& out) const {
	if(first.val == Value::none)
		out << "None (bottom)\n";
	else {
		#ifdef STATE_MULTILINE
			#define CLP_START "\t"
			#define CLP_END "\n"
			//out << "{\n";
		#else
			#define CLP_START ""
			#define CLP_END ", "
			out << "{";
		#endif
		// tmp registers
		/*for(int i = 0; i < tmpreg.length(); i++){
			Value val = tmpreg[i];
			if (val.kind() == VAL)
				out << CLP_START << "t" << i << " = " << val << CLP_END;
		}*/
		// registers
		for(int i = 0; i < registers.length(); i++){
			Value val = registers[i];
			if (val.kind() == VAL)
				out << CLP_START << "r" << i << " = " << val << CLP_END;
		}
		// memory
		for(Node *cur = first.next; cur; cur = cur->next) {
			out << CLP_START << Address(cur->addr);
			out << " = " << cur->val << CLP_END;
		}
		#ifndef STATE_MULTILINE 
		out << "}";
		#endif
	}
}
 	
/**
 * Return a stored value
 * @param addr is the addresse to get the value of. The kind of the value
 *        can be REG for a register, or VAL for memory. The address is
 *        considered as constant, the lower() attribute is the value.
 * @return the stored value
*/
const Value& State::get(const Value& addr) const {
	Node * cur;
	ASSERT(OCLP_IS_CST(addr)); // we assume that addr is a constant...
	if(addr.kind() == REG){
		// Tmp Registers
		if (addr.lower() < 0)
			if ((-addr.lower()) < tmpreg.length())
				return tmpreg[-addr.lower()];
			else
				return Value::all;
		// Real registers
		else if (addr.lower() < registers.length())
			return registers[addr.lower()];
		else
			return Value::all;
	} else {
		// Memory
		for(cur = first.next; cur && cur->addr < addr.lower(); cur = cur->next) ;
		if(cur && cur->addr == addr.lower())
			return cur->val;
		return first.val;
	}
}
 
const State State::EMPTY(Value::none), State::FULL(Value::all);
io::Output& operator<<(io::Output& out, const State& state) { state.print(out); return out; }

} //clp

/**
 * Definition of the abstract interpretation problem to make a CLP analysis
 */
class ClpProblem {
public:
	typedef clp::State Domain;
	 
	typedef ClpProblem Problem;
	Problem& getProb(void) { return *this; }
	
	ClpProblem(void): last_max_iter(0), specific_analysis(false), pack(NULL),
					   _nb_inst(0), _nb_sem_inst(0),
					   _nb_set(0), _nb_top_set(0), _nb_store(0), 
					   _nb_top_store(0), _nb_top_store_addr(0),
					   _nb_load(0), _nb_load_top_addr(0),
					   _nb_filters(0), _nb_top_filters(0) {}
	
	/* Initialize a register in the init state */
	void initialize(const hard::Register *reg, const Address& address) {
		clp::Value v;
		v = clp::Value(clp::VAL, address.offset());
		TRACEP(cerr << "init:: r" << reg->platformNumber() << " <- " << v.lower() << "\n");
		set(_init, reg->platformNumber(), v);
	}
	
	/** Provides the Bottom value of the Abstract Domain */
	inline const Domain& bottom(void) const { return clp::State::EMPTY; }
	
	/** Provides the entry state of the program */
	inline const Domain& entry(void) const {
		TRACEP(cerr << "entry() = " << _init << io::endl);
		return _init;
	}
	/**
	 * The Least Upper Bound of two elements a and b.
	 * This will be used, among other things, for performing the junction
	 * between two CFG paths.
	*/
	inline void lub(Domain &a, const Domain &b) const {
		a.join(b);
	}
	
	/**
	 * The windening operation
	 * This will be used to perform the junction between to iteration of
	 * a loop.
	*/
	inline void widening(Domain &a, Domain b) const{
		TRACEP(cerr << "Widening");
		TRACEP(cerr << a << "\n, " << b << ") = ");
		a.widening(b, last_max_iter);
		//DEBUG: print the state after the widening
		TRACEP(a.print(cerr));
		TRACEP(cerr << io::endl);
	}
	
	/**
	 * Update the domain in a way specific of the edge (for filtering purpose)
	*/
	inline void updateEdge(Edge *edge, Domain &dom){
		BasicBlock *source = edge->source();
		TRACEP(cerr << "Update edge from BB#" << source->number());
		TRACEP(cerr << " to BB#" << edge->target()->number() << " [taken=");
		TRACEP(cerr << (edge->kind() == Edge::TAKEN) << "]\n");
		if(se::REG_FILTERS.exists(source)){
			TRACEP(cerr << "\tApply filter on this edge!\n");
			Vector<se::SECmp *> reg_filters = se::REG_FILTERS(source);
			Vector<se::SECmp *> addr_filters = se::ADDR_FILTERS(source);
			for (int i=0; i < reg_filters.length(); i++){
				se::SECmp *filter; 
				// if the branch is taken, 'not' the filter
				if(edge->kind() == Edge::TAKEN)
					filter = reg_filters[i]->logicalNot();
				else
					filter = reg_filters[i]->copy();
				// apply the filter
				TRACEP(cerr << "\tFilter: " << filter->asString() << ' ');
				//FIXME: check in a() and b() are valid pointers
				clp::Value rval = filter->a()->val();
				clp::Value r = clp::Value(clp::REG, rval.lower(), 0, 0);
				clp::Value v = dom.get(r);
				TRACEP(v.print(cerr));
				TRACEP(cerr << " -> ");
				applyFilter(v, filter->op(), filter->b()->val());
				TRACEP(v.print(cerr));
				TRACEP(cerr << '\n');
				dom.set(r, v);
				_nb_filters++;
				if (filter->b()->val() == clp::Value::all)
					_nb_top_filters++;
				delete filter;
			}
			for (int i=0; i < addr_filters.length(); i++){
				se::SECmp *filter; 
				// if the branch is taken, 'not' the filter
				if(edge->kind() == Edge::TAKEN)
					filter = addr_filters[i]->logicalNot();
				else
					filter = addr_filters[i]->copy();
				// apply the filter
				TRACEP(cerr << "\tFilter: " << filter->asString() << ' ');
				//FIXME: check in a() and b() are valid pointers
				clp::Value a = filter->a()->val();
				clp::Value v = dom.get(a);
				TRACEP(v.print(cerr));
				// if we are from a loop header, widen the value
				TRACEP(cerr << " -> ");
				applyFilter(v, filter->op(), filter->b()->val());
				TRACEP(v.print(cerr));
				TRACEP(cerr << '\n');
				dom.set(a, v);
				_nb_filters++;
				if (filter->b()->val() == clp::Value::all)
					_nb_top_filters++;
				delete filter;
			}
		
		}
	}
	
	/** This function does the assignation of a state to another. */
	inline void assign(Domain &a, const Domain &b) const { a = b; }
	/** This functions tests two states for equality. */
	inline bool equals(const Domain &a, const Domain &b) const {
		return a.equals(b);
	}
	
	inline void enterContext(Domain &dom, BasicBlock *header, util::hai_context_t ctx) { }
	inline void leaveContext(Domain &dom, BasicBlock *header, util::hai_context_t ctx) { }
	
	/**
	 * This function update the state by applying a basic block.
	 * It gives the output state, given the input state and a pointer to the
	 * basic block.
	*/
	void update(Domain& out, const Domain& in, BasicBlock* bb) {
		int pc;
		out.copy(in);
		Domain *state;
		bool has_if = false;
		clp::ClpStatePack::InstPack *ipack = NULL;
		TRACEU(cerr << "update(BB" << bb->number() << ", " << in << ")\n");
		// save the input state in the basic block, join with an existing state
		// if needed
		if(!specific_analysis){
				CLP_STATE_IN(bb) = in;
		}
		if (LOOP_HEADER(bb)){
			TRACEU(cerr << "\tThis BB is a loop header.\n");
			if (MAX_ITERATION.exists(bb)){
				last_max_iter = MAX_ITERATION(bb);
				TRACEU(cerr << "\tFlow facts available: max iter=" << last_max_iter << '\n');
			} else {
				last_max_iter = -1;
			}
		}
		for(BasicBlock::InstIterator inst(bb); inst; inst++) {
			TRACEI(cerr << '\t' << inst->address() << ": ";
			inst->dump(cerr); cerr << io::endl);
			
			_nb_inst++;
			
			// get instructions
			b.clear();
			inst->semInsts(b);
			pc = 0;
			state = &out;
			
			if(specific_analysis && pack != NULL){
				ipack = pack->newPack(inst->address());
			}
			
			// perform interpretation
			while(true) {
				
				// interpret current
				while(pc < b.length()) {
					sem::inst& i = b[pc];
					
					_nb_sem_inst++;
					
					TRACEI(cerr << "\t\t" << i << io::endl);
					switch(i.op) {
					case sem::BRANCH:
					case sem::TRAP:
					case sem::CONT:
						pc = b.length();
						TRACEI(cerr << "\t\tcut\n");
						break;
					case sem::IF:
						todo.push(pair(pc + i.b() + 1, new Domain(*state)));
						has_if = true;
						break;
					case sem::NOP: break;
					case sem::LOAD: {
							_nb_load++;
							clp::Value addrclp = get(*state, i.a());
							//int bitsize = i.b() * 8;
							if (addrclp == clp::Value::all){
								set(*state, i.d(), addrclp);
								_nb_load_top_addr++;
							} else if (addrclp.mtimes() < 42){
								// if the addr is not cst, load only if
								// there is less than 42 values to join
								clp::Value addr(clp::VAL, addrclp.lower());
								set(*state, i.d(), state->get(addr));
								for(unsigned int m = 1; m <= addrclp.mtimes(); m++){
									//cerr << "load for m=" << m << '\n';
									// join other values with the first
									clp::Value addr(clp::VAL,
										addrclp.lower() + addrclp.delta() * m);
									clp::Value val = get(*state, i.d());
									val.join(state->get(addr));
									set(*state, i.d(), val);
								}
							} else {
								set(*state, i.d(), clp::Value::all);
							}
							#ifdef DATA_LOADER
							// if the value loaded is T, load from the process
							if ((get(*state, i.d()) == clp::Value::all) && *_data_min != 0){
								cerr << "DATA_LOADER: load gets a T -> ";
								cerr << "@[" << hex(*_data_min) << "<=";
								addrclp.print(cerr);
								cerr << "<" << hex(*_data_max) << "]";
								cerr << " -> ";
								cerr << ((*_data_min <= (OCLP_uintn_t)addrclp.start())&&((OCLP_uintn_t)addrclp.start() < *_data_max));
							}
							if (    (get(*state, i.d()) == clp::Value::all)			&&
								    (OCLP_IS_CST(addrclp))							&&
								    (*_data_min <= (OCLP_uintn_t)addrclp.start())	&&
								    ((OCLP_uintn_t)addrclp.start() < *_data_max)	){
								ASSERT(i.b() * 8 <= OCLP_NBITS);
								cerr << " -> loading data from process\n";
								OCLP_intn_t value;
								_process->get(addrclp.lower(), (char *)(&value), i.b());
								set(*state, i.d(), clp::Value(value));
							}
							if ((get(*state, i.d()) == clp::Value::all) && *_data_min != 0)
								cerr << '\n';
							#endif
						} break;
					case sem::STORE: {
							clp::Value addrclp = get(*state, i.a());
							//int bitsize = i.b() * 8;
							if (addrclp == clp::Value::all){
								state->set(addrclp, get(*state, i.d()));
								_nb_store++; _nb_top_store ++;
								_nb_top_store_addr++;
							} else if (addrclp.mtimes() < 42){
								// unroll the clp (only if less than 42 values)
								for(unsigned int m = 0; m <= addrclp.mtimes(); m++){
									clp::Value addr(clp::VAL,
										addrclp.lower() + addrclp.delta() * m);
									state->set(addr, get(*state, i.d()));
									_nb_store++;
									if (get(*state, i.d()) == clp::Value::all)
										_nb_top_store++;
								}
							} else {
								TRACEU(cerr << "Warning: STORE to ");
								TRACEU(addrclp.print(cerr));
								TRACEU(cerr << " : to many values, set memory");
								TRACEU(cerr << " to T.\n");
								state->set(clp::Value::all, get(*state, i.d()));
								_nb_store++; _nb_top_store++;
								_nb_top_store_addr++;
							}
						} break;
					case sem::SETP:
					case sem::CMP:
					case sem::CMPU:
					case sem::SCRATCH:
						set(*state, i.d(), clp::Value::all);
						break;
					case sem::SET: {
							clp::Value v = get(*state, i.a());
							set(*state, i.d(), v);
							_nb_set++;
							if (v == clp::Value::all)
								_nb_top_set++;
						} break;
					case sem::SETI: {
							clp::Value v(clp::VAL, i.cst());
							set(*state, i.d(), v);
						} break;
					case sem::ADD: {
							clp::Value v = get(*state, i.a());
							v.add(get(*state, i.b()));
							set(*state, i.d(), v);
						} break;
					case sem::SUB: {
							clp::Value v = get(*state, i.a());
							v.sub(get(*state, i.b()));
							set(*state, i.d(), v);
						} break;
					case sem::SHL: {
							clp::Value v = get(*state, i.a());
							v.shl(get(*state, i.b()));
							set(*state, i.d(), v);
						} break;
					case sem::SHR: case sem::ASR: {
							clp::Value v = get(*state, i.a());
							v.shr(get(*state, i.b()));
							set(*state, i.d(), v);
						} break;
					}
					//DEBUG: print the result of the instruction
					TRACEI(cerr << "\t\t -> " << *state << io::endl);
					
					if (specific_analysis && pack != NULL){
						ipack->append(*state);
					}
					
					pc++;
				}
				
				// pop next
				if(state != &out) {
					out.join(*state);
					delete state;
				}
				if(!todo)
					break;
				else {
					Pair<int, Domain *> p = todo.pop();
					pc = p.fst;
					state = p.snd;
				}
				
			}
			TRACEI(cerr << "\t-> " << out << io::endl);
		}
		
		if (specific_analysis)
			return;
		
		// save the output state in the basic block
		CLP_STATE_OUT(bb) = out;
		TRACEU(cerr << ">>>\tout = " << out << io::endl);
		
		// if the block has an IF instruction
		if(has_if /*&& ! se::REG_FILTERS.exists(bb)*/){ // re-evaluate filters, because values can change!
			//TODO: delete 'old' reg_filters if needed
			//use Symbolic Expressions to get filters for this basic block
			TRACEP(cerr << "> IF detected, getting filters..." << io::endl);
			se::getFilters(bb);
		}
	}
	
	/**
	 * Get the content of a register
	 * @param state is the state from wich we want information
	 * @param i is the identifier of the register
	 * @return the value of the register
	 */
	const clp::Value& get(const clp::State& state, int i) {
		clp::Value addr(clp::REG, i);
		return state.get(addr);
	}
	
	/**
	 * Set a register to the given value
	 * @param state is the initial state
	 * @param i is the regsiter identifier (i < 0 for temp registers)
	 * @param v is the value to set
	 * @return the new state
	*/
	const void set(clp::State& state, int i, const clp::Value& v) {
		clp::Value addr(clp::REG, i);
		return state.set(addr, v);
	}
	
	void fillPack(BasicBlock* bb, clp::ClpStatePack *empty_pack){
		specific_analysis = true;
		pack = empty_pack;
		clp::State output;
		clp::State input = CLP_STATE_IN(*bb);
		update(output, input, bb);
		pack = NULL;
		specific_analysis = false;
	}
	
	/**
	 * Return various statistics about the analysis
	*/
	inline OCLP_STAT_UINT get_nb_inst(void){ return _nb_inst; }
	inline OCLP_STAT_UINT get_nb_sem_inst(void){ return _nb_sem_inst; }
	inline OCLP_STAT_UINT get_nb_set(void){ return _nb_set; }
	inline OCLP_STAT_UINT get_nb_top_set(void){ return _nb_top_set; }
	inline OCLP_STAT_UINT get_nb_store(void){ return _nb_store; }
	inline OCLP_STAT_UINT get_nb_top_store(void){ return _nb_top_store; }
	inline OCLP_STAT_UINT get_nb_top_store_addr(void){return _nb_top_store_addr;}
	inline OCLP_STAT_UINT get_nb_load(void){return _nb_load;}
	inline OCLP_STAT_UINT get_nb_load_top_addr(void){return _nb_load_top_addr;}
	inline OCLP_STAT_UINT get_nb_filters(void){ return _nb_filters;}
	inline OCLP_STAT_UINT get_nb_top_filters(void){ return _nb_top_filters;}
	
	#ifdef DATA_LOADER
	inline void set_data_space(address_t data_min, address_t data_max, Process* p){
		_data_min = data_min;
		_data_max = data_max;
		_process = p;
	}
	#endif
	
private:
	clp::State _init;
	sem::Block b;
	genstruct::Vector<Pair<int, Domain *> > todo;
	
	int last_max_iter;
	
	/* attribute for specific analysis / packing */
	bool specific_analysis;
	clp::ClpStatePack *pack;
	
	#ifdef DATA_LOADER
	address_t _data_min, _data_max;
	Process* _process;
	#endif
	
	/* attributes for statistics purpose */
	OCLP_STAT_UINT _nb_inst;
	OCLP_STAT_UINT _nb_sem_inst;
	OCLP_STAT_UINT _nb_set;
	OCLP_STAT_UINT _nb_top_set;
	OCLP_STAT_UINT _nb_store;
	OCLP_STAT_UINT _nb_top_store;
	OCLP_STAT_UINT _nb_top_store_addr;
	OCLP_STAT_UINT _nb_load;
	OCLP_STAT_UINT _nb_load_top_addr;
	OCLP_STAT_UINT _nb_filters;
	OCLP_STAT_UINT _nb_top_filters;
};

// CLPStateCleaner
class CLPStateCleaner: public Cleaner {
public:
	inline CLPStateCleaner(CFG *_cfg) {cfg = _cfg;}
	//virtual ~CLPStateCleaner(void) {}
	virtual void clean(void) {
		for(CFG::BBIterator bbi(cfg); bbi; bbi++){
			CLP_STATE_IN(*bbi).remove();
			CLP_STATE_OUT(*bbi).remove();
		}
	}
private:
	CFG *cfg;
};

/**
 * @class ClpAnalysis
 *
 * This analyzer tracks values in the form of a CLP.
 *
 * @par Widening
 *
 * The widening of this analysis is performed using filtering from symbolic
 * expressions.
 *
 * @par Provided Features
 * @li @ref otawa::CLP_ANALYSIS_FEATURE
 *
 * @par Required Features
 * @li @ref otawa::LOOP_INFO_FEATURE
 * @li @ref otawa::ipet::FLOW_FACTS_FEATURE
 * @li @ref otawa::VIRTUALIZED_CFG_FEATURE
 *
*/ 
ClpAnalysis::ClpAnalysis(void): Processor("otawa::ClpAnalysis", Version(0, 1, 0)),
								_nb_inst(0), _nb_sem_inst(0), _nb_set(0), 
								_nb_top_set(0), _nb_store(0), _nb_top_store(0),
								_nb_filters(0), _nb_top_filters(0){
	require(LOOP_INFO_FEATURE);
	require(ipet::FLOW_FACTS_FEATURE);
	require(VIRTUALIZED_CFG_FEATURE);
	provide(CLP_ANALYSIS_FEATURE);
}
 
 
/**
 * Perform the analysis by processing the workspace
 * @param ws the workspace to be processed
 */
void ClpAnalysis::processWorkSpace(WorkSpace *ws) {
	typedef WideningListener<ClpProblem> ClpListener;
	typedef WideningFixPoint<ClpListener> ClpFP;
	typedef HalfAbsInt<ClpFP> ClpAI;
	 
	// get the entry
	const CFGCollection *coll = INVOLVED_CFGS(ws);
	ASSERT(coll);
	CFG *cfg = coll->get(0);
	
	// set the cleaner
	CLPStateCleaner *cleaner = new CLPStateCleaner(cfg);
	addCleaner(CLP_ANALYSIS_FEATURE, cleaner);
	
	ClpProblem prob;
	
	#ifdef DATA_LOADER
	// find the address interval of data from the process
	address_t data_min = 0, data_max = 0;
	Process* p = ws->process();
	for(Process::FileIter pfi(p); pfi; pfi++){
		for(File::SegIter segi(pfi); segi; segi++){
			if(data_min == data_max){
				data_min = segi->address();
				data_max = segi->topAddress();
			} else {
				if(segi->address() < data_min)
					data_min = segi->address();
				if(segi->topAddress() > data_max)
					data_max = segi->topAddress();
			}
		}
	}
	prob.set_data_space(data_min, data_max, p);
	#endif
	
	// perform the analysis
	if(isVerbose())
		log << "FUNCTION " << cfg->label() << io::endl;
	for(int i = 0; i < inits.count(); i++){
		prob.initialize(inits[i].fst, inits[i].snd);
	}
	ClpListener list(ws, prob);
	ClpFP fp(list);
	ClpAI cai(fp, *ws);
	cai.solve(cfg);
	_nb_inst = prob.get_nb_inst();
	_nb_sem_inst = prob.get_nb_sem_inst();
	_nb_set = prob.get_nb_set();
	_nb_top_set = prob.get_nb_top_set();
	_nb_store = prob.get_nb_store();
	_nb_top_store = prob.get_nb_top_store();
	_nb_top_store_addr = prob.get_nb_top_store_addr();
	_nb_load = prob.get_nb_load();
	_nb_load_top_addr = prob.get_nb_load_top_addr();
	_nb_filters = prob.get_nb_filters();
	_nb_top_filters = prob.get_nb_top_filters();
}
 
 
/**
 * Build the initial configuration of the Analysis fro a property list
 * @param props the property list
 */
void ClpAnalysis::configure(const PropList &props) {
	Processor::configure(props);
	for(Identifier<init_t>::Getter init(props, INITIAL); init; init++)
		inits.add(init);
}
 

/**
 * This features ensure that the clp analysis has been identified.
 * @par Default Processor
 * @li @ref otawa::ClpAnalysis
 */
Feature<ClpAnalysis> CLP_ANALYSIS_FEATURE("otawa::CLP_ANALYSIS_FEATURE");

/**
 * Put on a basic block, it's the CLP state at the begining of the block
*/
Identifier<clp::State> CLP_STATE_IN("otawa::CLP_STATE_IN");

/**
 * Put on a basic block, it's the CLP state at the end of the block
*/
Identifier<clp::State> CLP_STATE_OUT("otawa::CLP_STATE_OUT");

namespace clp {

// ClpStatePack
/** Constructor of a new ClpStatePack.
*	@param bb is the BasicBlock to be analysed.
*	
*	A ClpStatePack must be constructed after the run of the ClpAnalysis.
*	This constructor will use the input state of the BasicBlock, and run again
*	the analysis until the end of the block.
*	
*	The state for each instruction and semantic instruction will be saved inside
*	the pack.
*/
ClpStatePack::ClpStatePack(BasicBlock *bb): _bb(bb), _packs(){
	ASSERT(CLP_STATE_IN.exists(*bb));
	ClpProblem prob;
	prob.fillPack(_bb, this);
}
/** Destructor for ClpStatePack */
ClpStatePack::~ClpStatePack(void){
	while(!_packs.isEmpty()){
		InstPack *p = _packs.pop();
		delete p;
	}
}
/** Destructor for InstPack */
ClpStatePack::InstPack::~InstPack(void){
	while(!_states.isEmpty()){
		clp::State *st = _states.pop();
		delete st;
	}
}
/** Add a new state at the end of this pack.
*	@param state the state to be added.
*/
void ClpStatePack::InstPack::append(clp::State &state){
	clp::State *st = new clp::State(state);
	_states.add(st);
}
/** @return the CLP state after the given instruction
*	@param instruction is the address of the instruction to get the state of.
*/
clp::State ClpStatePack::state_after(address_t instruction){
	for(PackIterator packs = getIterator(); packs; packs++){
		InstPack *ipack = (*packs);
		if (ipack->inst_addr() == instruction)
			return ipack->outputState();
	}
	return clp::State::EMPTY; // FIXME: we should raise an exception?
}
/** @return the CLP state after the given semantic instruction
*	@param instruction is the address of the instruction where the semantic
*		instruction is.
*	@param sem is the index (starting from 0) of the semantic instruction inside
*		the block corresponding to the machine instruction.
*/
clp::State ClpStatePack::state_after(address_t instruction, int sem){
	for(PackIterator packs = getIterator(); packs; packs++){
		InstPack *ipack = (*packs);
		if (ipack->inst_addr() == instruction)
			return *(ipack->_states[sem]);
	}
	return clp::State::EMPTY; // FIXME: we should raise an exception?
}
/** @return the CLP state before the given instruction
*	@param instruction is the address of the instruction to get the state before.
*/
clp::State ClpStatePack::state_before(address_t instruction){
	clp::State last_state = CLP_STATE_IN(_bb);
	for(PackIterator packs = getIterator(); packs; packs++){
		InstPack *ipack = (*packs);
		if (ipack->inst_addr() == instruction)
			return last_state;
		last_state = ipack->outputState();
	}
	return last_state;
}
/** @return the CLP state before the given semantic instruction
*	@param instruction is the address of the instruction where the semantic
*		instruction is.
*	@param sem is the index (starting from 0) of the semantic instruction inside
*		the block corresponding to the machine instruction.
*/
clp::State ClpStatePack::state_before(address_t instruction, int sem){
	clp::State last_state = CLP_STATE_IN(_bb);
	for(PackIterator packs = getIterator(); packs; packs++){
		InstPack *ipack = (*packs);
		if (ipack->inst_addr() == instruction){
			if ( sem == 0)
				return last_state;
			else
				return *(ipack->_states[sem - 1]);
		}
		if (! ipack->isEmpty())
			last_state = ipack->outputState();
	}
	return last_state;
}

/** Add a new instruction pack inside the ClpStatePack. */
ClpStatePack::InstPack* ClpStatePack::newPack(address_t inst){
	InstPack *ipack = new InstPack(inst);
	_packs.add(ipack);
	return ipack;
}

} //clp
/**
 * @class DeadCodeAnalysis
 *
 * This analyzer add the NEVER_TAKEN identifer on edges which are never taken.
 *
 * @par Provided Features
 * @li @ref otawa::DEAD_CODE_ANALYSIS_FEATURE
 *
 * @par Required Features
 * @li @ref otawa::CLP_ANALYSIS_FEATURE
 */
DeadCodeAnalysis::DeadCodeAnalysis(void): Processor("otawa::DeadCodeAnalysis", Version(0, 1, 0)) {
	require(CLP_ANALYSIS_FEATURE);
	provide(DEAD_CODE_ANALYSIS_FEATURE);
}

void DeadCodeAnalysis::processWorkSpace(WorkSpace *ws){
	const CFGCollection *coll = INVOLVED_CFGS(ws);
	ASSERT(coll);
	CFG *cfg = coll->get(0);
	/* for each bb */
	for(otawa::CFG::BBIterator bbi(cfg); bbi; bbi++){
		clp::State st = CLP_STATE_OUT(*bbi);
		/* if the bb state is None : */
		if (st == clp::State::EMPTY){
			/* mark all (in|out) edges as never taken */
			for(BasicBlock::InIterator edge(*bbi); edge; edge++)
				NEVER_TAKEN(*edge) = true;
			for(BasicBlock::OutIterator edge(*bbi); edge; edge++)
				NEVER_TAKEN(*edge) = true;
		} else {
			/* mark all (in|out) edges as not never taken */
			for(BasicBlock::InIterator edge(*bbi); edge; edge++)
				if (!NEVER_TAKEN.exists(*edge))
					NEVER_TAKEN(*edge) = false;
			for(BasicBlock::OutIterator edge(*bbi); edge; edge++)
				if (!NEVER_TAKEN.exists(*edge))
					NEVER_TAKEN(*edge) = false;
		}
	}
}

Feature<DeadCodeAnalysis> DEAD_CODE_ANALYSIS_FEATURE("otawa::DEAD_CODE_ANALYSIS_FEATURE");
Identifier<bool> NEVER_TAKEN("otawa::NEVER_TAKEN");

} //otawa

