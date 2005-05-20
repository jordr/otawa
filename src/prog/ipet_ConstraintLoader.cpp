/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	prog/ipet_ConstraintLoader.cpp -- ConstraintLoader class implementation.
 */

#include <stdio.h>
#include <elm/string/StringBuffer.h>
#include <otawa/ipet/IPET.h>
#include <otawa/ipet/ConstraintLoader.h>
#include <otawa/cfg.h>
#include "ExpNode.h"

// Externals
extern FILE *ipet_in;

namespace otawa {

// NormNode class
class NormNode {
	double coef;
	ilp::Var *var;
	NormNode *nxt;
public:
	inline NormNode(double coefficient, ilp::Var *variable = 0, NormNode *next = 0)
		: coef(coefficient), var(variable), nxt(next) {
		};
	inline double coefficient(void) const
		{ return coef; };
	inline ilp::Var *variable(void) const
		{ return var; };
	inline NormNode *next(void) const
		{ return nxt; };
	inline NormNode *append(NormNode *to_append) {
		NormNode *cur;
		for(cur = this; cur->nxt; cur = cur->nxt) ;
		cur->nxt = to_append;
		return this;
	};
	inline bool isConstant(void) const
		{ return var == 0 && !nxt; };
	inline bool isSimple(void) const
		{ return !nxt; };
	inline NormNode *multiply(double cst) {
		for(NormNode *cur = this; cur; cur = cur->nxt)
			cur->coef *= cst;
		return this;
	};
};


// NormalizationException class
class NormalizationException {
};


/**
 * @class ConstraintLoader
 * This code processor allows loading IPET constraint from an external file.
 * !!TODO!!
 * File format documentation here !
 */


/**
 * This property identifier is used for passing specific file to load to
 * the @ref ConstraintLoader (argument of type @ref elm::String).
 */
Identifier ConstraintLoader::ID_Path("otawa.ipet.path");

/**
 * Find the BB matching the given address.
 * @param addr	Address of BB to find.
 * @return		Found BB or null.
 */
BasicBlock *ConstraintLoader::getBB(address_t addr) {
	BasicBlock *bb = bbs.get(addr, 0);
	if(!bb) {
		bb = BasicBlock::findBBAt(fw, addr);
		if(!bb) {
			out << "ERROR: cannot find basic block at " << addr << ".\n"; 
			return 0;
		}
		bbs.put(addr, bb);
	}
	return bb;
}


/**
 * For internal use only.
 */
bool ConstraintLoader::newBBVar(CString name, address_t addr) {
	BasicBlock *bb = getBB(addr);
	if(!bb)
		return false;
	ilp::Var *var = bb->get<ilp::Var *>(IPET::ID_Var, 0);
	if(!var)
		out << "ERROR: variable " << name << " of basic block at " << addr
			<< " is not defined.\n";
	else
		vars.put(name, var);
	return true;
}


/**
 * For internal use only.
 */
bool ConstraintLoader::newEdgeVar(CString name, address_t src, address_t dst) {
	
	// Find basic blocks
	BasicBlock *src_bb = getBB(src);
	if(!src_bb)
		return false;
	BasicBlock *dst_bb = getBB(dst);
	if(!dst_bb)
		return false;
	
	// Find edge 
	for(Iterator<Edge *> edge(src_bb->outEdges()); edge; edge++) {
		if(edge->target() == dst_bb) {
			ilp::Var *var = edge->get<ilp::Var *>(IPET::ID_Var, 0);
			if(var) {
				vars.put(name, var);
				return var;
			}
			else
				cout << "ERROR: variable " << name
					<< " of edge from basic block " << src
					<< " to basic block " << dst
					<< " is not defined.\n";
		}
	}
	
	// Not found error
	out << "ERROR: no edge from basic block " << src
		<< " to basic block " << dst << ".\n";
	return false;
}


/**
 * For internal use only.
 */
ilp::Var *ConstraintLoader::getVar(CString name) {
	ilp::Var *var = vars.get(name, 0);
	if(!var)
		cout << "ERROR: variable \"" << name << "\" is not defined.\n";
	return var;
}


/**
 * Add a constraint to the current ILP system.
 * @param left		Left side of the equation.
 * @param t			Comparator of the equation.
 * @param right		Right side of the equation.
 * @return
 */
bool ConstraintLoader::addConstraint(ExpNode *left, ilp::Constraint::comparator_t t,
ExpNode *right) {
	try {
		NormNode *norm = normalize(left, 1);
		norm->append(normalize(right, -1));
		ilp::Constraint *cons = system->newConstraint(t);
		for(; norm; norm = norm->next())
			cons->add(norm->coefficient(), norm->variable());
		return true;
	}
	catch(NormalizationException e) {
		out << "ERROR: expression cannot be reduced !\n";
		return false;
	}
}


/**
 * Normalize the given expression.
 * @param node	Expression to normalize.
 * @param mult	Multiplier to apply.
 * @return		Normalized expression.
 */	
NormNode *ConstraintLoader::normalize(ExpNode *node, double mult) {
	assert(node);
	NormNode *arg1, *arg2;

	switch(node->kind()) {

	// Constant		
	case ExpNode::CST:
		return new NormNode(mult * node->cst());

	// Variable
	case ExpNode::VAR:
		return new NormNode(mult, node->var());
	
	// Positive
	case ExpNode::POS:
		return normalize(node->arg(), mult);
	
	// Negative
	case ExpNode::NEG:
		return normalize(node->arg(), -mult);
	
	// Subtraction
	case ExpNode::SUB:
		arg1 = normalize(node->arg1(), mult);
		arg2 = normalize(node->arg2(), -mult);
		goto process_sum;

	// Addition
	case ExpNode::ADD:
		arg1 = normalize(node->arg1(), mult);
		arg2 = normalize(node->arg2(), mult);
	process_sum:
		if(arg1->isConstant() && arg2->isConstant())
			return new NormNode(arg1->coefficient() + arg2->coefficient());
		else
			return arg1->append(arg2);
	
	// Multiplication
	case ExpNode::MUL:
		arg1 = normalize(node->arg1(), mult);
		arg2 = normalize(node->arg2(), mult);
		if(arg1->isConstant())
			return arg2->multiply(arg1->coefficient());
		else if(arg2->isConstant())
			return arg1->multiply(arg2->coefficient());
		else
			throw new NormalizationException();
	
	// Division
	case ExpNode::DIV:
		if(arg2->isConstant())
			return arg1->multiply(1 / arg2->coefficient());
		else
			throw NormalizationException();
	
	// Too bad !
	default:
		assert(0);
	}
}


/**
 */
void ConstraintLoader::configure(PropList& props) {
	path = props.get<String>(ID_Path, "");
	CFGProcessor::configure(props);
}


/**
 * <p>Read the constraint file and add it to the current ILP system.</p>
 * <p>The read file path is taken from configuration if available, or built
 * from the binary file path with ".ipet" appended.</p>
 */
void ConstraintLoader::processCFG(FrameWork *_fw, CFG *cfg) {
	
	// Initialization
	fw = _fw;
	system = cfg->get<ilp::System *>(IPET::ID_System, 0);
	if(!system) {
		out << "ERROR: no ILP system available on this CFG.\n";
		return;
	}
	
	// Select the file
	if(!path) {
		Iterator<File *> file(*fw->files());
		elm::StringBuffer buffer;
		buffer.put(file->name());
		buffer.put(".ipet");
		path = buffer.toString();
	}
	
	// Open the file
	ipet_in = fopen(&path.toCString(), "r");
	if(!ipet_in) {
		out << "ERROR: cannot open the constraint file \"" << path << "\".\n";
		return;
	}
	
	// Perform the parsing
	ipet_parse(this);
	
	// Close all
	fclose(ipet_in);
}

} // otawa
