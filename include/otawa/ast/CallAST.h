/*
 *	$Id$
 *	Copyright (c) 2003, Institut de Recherche en Informatique de Toulouse.
 *
 *	otaw/ast/CallAST.h -- interface for CallAST class.
 */
#ifndef OTAWA_AST_CALL_AST_H
#define OTAWA_AST_CALL_AST_H

#include <otawa/ast/BlockAST.h>
#include <otawa/ast/FunAST.h>

namespace otawa {

// CallAST Class
class CallAST: public BlockAST {
	AutoPtr<FunAST> fun;
public:
	CallAST(Inst *callee, AutoPtr<FunAST> fun);
	CallAST(FrameWork *fw, Inst *callee, Inst *called);
	inline AutoPtr<FunAST> function(void) const { return fun; };
	
	// AST overload
	virtual ast_kind_t kind(void) const { return AST_Call; };
	virtual AutoPtr<CallAST> toCall(void) { return this; };
};
	
} // otawa

#endif // OTAWA_AST_CALL_AST_H
