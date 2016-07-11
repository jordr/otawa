/*
 *	icat3::MustPersDomain class interface
 *	Copyright (c) 2016, IRIT UPS.
 *
 *	This file is part of OTAWA
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */


#include "MustPersDomain.h"

namespace otawa { namespace icat3 {

/**
 * @class MustPersDomain
 * Abstract Interpretation domain implementing the co-analysis MUST and PERS
 * for L1 instruction cache.
 * @ingroup icat3
 */

/**
 * Build the MUST-PERS analysis domain.
 * @param coll		L-block collection.
 * @param set		Current cache state.
 * @param must_init	Initial ACS for MUST analysis.
 */
MustPersDomain::MustPersDomain(const LBlockCollection& coll, int set, const MustDomain::t *must_init, const ACS *pers_init)
	:	_must(coll, set, must_init),
		_pers(coll, set, pers_init),
		n(coll[set].count()),
		_bot(_must.bot(), _pers.bot()),
		_top(_must.top(), _pers.top()),
		_init(_must.init(), _pers.init())
	{ }

/**
 * Print the join state (MUST, PERS).
 * @param a		Value to display.
 * @param out	Output to use.
 */
void MustPersDomain::print(const t& a, io::Output& out) const {
	out << '(';
	_must.print(a.must, out);
	out << ", ";
	_pers.print(a.pers, out);
	out << ')';
}

/**
 * Join two ACS.
 * @param a	ACS to join in.
 * @param b	ACS to join with.
 */
void MustPersDomain::join(t& a, const t& b) {
	_must.join(a.must, b.must);
	_pers.join(a.pers, b.pers);
}

/**
 * Update the ACS with an instruction cache operation.
 * @param o	Operation to perform.
 * @param a	ACS to update.
 */
void MustPersDomain::update(const icache::Access& o, t& a) {
	_must.update(o, a.must);
	_pers.update(o, a.pers);
}

/**
 * Update an ACS with a list of access operation.
 * @param os	Instruction cache access operation.
 * @param a		Acces to update.
 */
void MustPersDomain::update(const Bag<icache::Access>& os, t& a) {
	for(int i = 0; i < os.size(); i++)
		update(os[i], a);
}

/**
 * Update an ACS from an edge transition.
 * @param v		Source block.
 * @param e		Traversed edge.
 * @param a		ACS to update.
 */
void MustPersDomain::update(Block *v, Edge *e, t& a) {
	const Bag<icache::Access>& os = icache::ACCESSES(v);
	update(os, a);
	const Bag<icache::Access>& os2 = icache::ACCESSES(v);
	update(os2, a);
}

/**
 * Test if two ACS are equal.
 * @param a	First ACS.
 * @param b	Second ACS.
 * @return	True for equality, false else.
 */
bool MustPersDomain::equals(const t& a, const t& b) {
	return _must.equals(a.must, b.must) && _pers.equals(a.pers, b.pers);
}

} }	// otawa::icat3
