/*
 *	$Id$
 *	Copyright (c) 2003, Institut de Recherche en Informatique de Toulouse.
 *
 *	otaw/ast/ASTInfo.h -- interface for ASTInfo class.
 */
#ifndef OTAWA_AST_AST_INFO_H
#define OTAWA_AST_AST_INFO_H

#include <elm/datastruct/HashTable.h>
#include <elm/datastruct/Vector.h>
#include <otawa/ast/FunAST.h>

// Extern
int heptane_parse(void);

namespace otawa {

// ASTInfo class
class ASTInfo: public PropList {
	friend class CallAST;
	friend class FunAST;
	friend class GenericProperty<ASTInfo *>;
	friend int ::heptane_parse(void);
	elm::datastruct::Vector<FunAST *> funs;
	elm::datastruct::HashTable<String, FunAST *> _map;
	void add(FunAST *fun);
	ASTInfo(WorkSpace *fw);
public:
	~ASTInfo(void);
	static Identifier<ASTInfo *> ID;
	static ASTInfo *getInfo(WorkSpace *fw);
	FunAST *getFunction(Inst *inst);
	inline elm::datastruct::Map<String, FunAST *>& map(void) { return _map; };
	inline elm::Collection< FunAST *>& functions(void) { return funs; };
};
	
} // otawa

#endif // OTAWA_AST_AST_INFO_H
