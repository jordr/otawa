/*
 * $Id$
 * Copyright (c) 2005 IRIT-UPS
 *
 * include/util/Dominance.h -- Dominance class interface.
 */
#ifndef OTAWA_UTIL_DOMINANCE_H
#define OTAWA_UTIL_DOMINANCE_H

#include <elm/Collection.h>
#include <otawa/proc/CFGProcessor.h>
#include <otawa/proc/Feature.h>

namespace otawa {

// External
class BasicBlock;
class BitSet;

// Dominance class
class Dominance: public CFGProcessor {
public:
	static void ensure(CFG *cfg);
	static bool dominates(BasicBlock *bb1, BasicBlock *bb2);
	static inline bool isDominated(BasicBlock *bb1, BasicBlock *bb2);
	static bool isLoopHeader(BasicBlock *bb);
	static void markLoopHeaders(CFG *cfg,
		elm::MutableCollection<BasicBlock *> *headers = 0);

	// Constructor
	Dominance(void);
	
	// CFGProcessor overload
	virtual void processCFG(FrameWork *fw, CFG *cfg);
};

// Features
extern Feature<Dominance> DOMINANCE_FEATURE;
extern Feature<Dominance> LOOP_HEADERS_FEATURE;

// Properties
extern GenericIdentifier<BitSet *> REVERSE_DOM;
extern GenericIdentifier<bool> LOOP_HEADER;

} // otawa

#endif // OTAWA_UTIL_DOMINANCE_H
