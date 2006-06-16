/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	otawa/prog/FrameWork.h -- interface for FrameWork class.
 */
#ifndef OTAWA_PROG_FRAMEWORK_H
#define OTAWA_PROG_FRAMEWORK_H

#include <elm/Collection.h>
#include <otawa/properties.h>
#include <otawa/prog/Process.h>

namespace otawa {

// Classes
class AST;
class ASTInfo;
class CFG;
class CFGInfo;
class File;
class Inst;
class Manager;
namespace hard {
	class Platform;
}
namespace ilp {
	class System;
}

// FrameWork class
class FrameWork: public Process {
	Process *proc;
protected:
	virtual Property *getDeep(const Identifier *id) { return proc->getProp(id); };
public:
	FrameWork(Process *_proc);
	~FrameWork(void);
	inline Process *process(void) const { return proc; };
	
	// Process overload
	virtual elm::Collection<File *> *files(void) { return proc->files(); };
	virtual File *createFile(void) { return proc->createFile(); };
	virtual File *loadFile(elm::CString path) { return proc->loadFile(path); };
	virtual hard::Platform *platform(void) { return proc->platform(); };
	virtual Manager *manager(void) { return proc->manager(); };
	virtual Inst *start(void) { return proc->start(); };
	virtual Inst *findInstAt(address_t addr) { return proc->findInstAt(addr); };
	virtual const hard::CacheConfiguration& cache(void);
	
	// CFG Management
	CFGInfo *getCFGInfo(void);
	CFG *getStartCFG(void);
	
	// AST Management
	ASTInfo *getASTInfo(void);
	
	// ILP support
	ilp::System *newILPSystem(bool max = true);
};

};	// otawa

#endif	// OTAWA_PROG_FRAMEWORK_H
