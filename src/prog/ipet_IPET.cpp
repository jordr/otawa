/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/prog/ipet_IPET.cpp -- IPET class implementation.
 */

#include <elm/util/MessageException.h>
#include <otawa/ipet/IPET.h>
#include <otawa/ilp.h>
#include <otawa/manager.h>

namespace otawa { namespace ipet {

using namespace ilp;
 
/**
 * @page ipet IPET Method
 * 
 * OTAWA provides many code processors supporting IPET approach:
 * @li @ref VarAssignment - assign ILP variable to each basic block and edge
 * of the graph.
 * @li @ref BasicConstraintBuilder - build constraints generated from the
 * program CFG,
 * @li @ref BasicObjectFunctionBuilder - build the object function to maximize
 * to get the WCET,
 * @li @ref ConstraintLoader - add constraints from an external file,
 * @li @ref FlowFactLoader - load and add constraints for the flow facts
 * (currently constante loop upper bounds only),
 * @li @ref WCETComputation - perform the WCET final computation.
 * 
 * Some processors are dedicated to pipeline-level analyses:
 * @li @ref TrivialBBTime - compute the execution time of a basic block by
 * applying a constant execution to each instruction,
 * @li @ref Delta - change the constraints and the object function to implement
 * the Delta method for inter-basic-block pipe-line effect support,
 * @li @ref ExeGraphBBTime - compute the execution time of basic block using
 * an execution graph.
 * 
 * Some processors are dedicated to cache analyses:
 * @li @ref CATBuilder - compute the cache category of each instruction
 * (always-miss, always-hit, first-miss, first-it),
 * @li @ref CATConstraintBuilder - add constraints to use instruction access
 * categories,
 * @li @ref CCGBuilder - build the Cache Conflict Graph for the instruction
 * cache,
 * @li @ref CCGConstraintBuilder - build the constraints using the CCG to
 * take in account instruction cache accesses,
 * @li @ref CCGObjectFunction - the CCG instruction cache approach requires
 * to use a special object function in order to get the WCET,
 * @li @ref TrivialDataCacheManager - consider each memory access as a miss.
 * 
 * These processors interacts using the following properties:
 * @li @ref TIME - execution time of a program part (usually a basic block), 
 * @li @ref VAR - ILP variable associated with a program component (usually
 * a basic block or an edge),
 * @li @ref SYSTEM - ILP system associated with a root CFG,
 * @li @ref WCET - WCET of a root CFG,
 * @li @ref EXPLICIT - used in configuration of code processor, cause the
 * generation of explicit variable names instead of numeric ones (default to false),
 * @li @ref RECURSIVE - cause the processors to work recursively (default to false).
 */


/**
 * This identifier is used for storing the time of execution in cycles (int)
 * of the program area it applies to.
 */
GenericIdentifier<int> TIME("ipet.time", -1);


/**
 * This identifier is used for storing in basic blocks and edges the variables
 * (otawa::ilp::Var *) used in ILP resolution.
 */
GenericIdentifier<ilp::Var *> VAR("ipet.var", 0);


/**
 * Identifier of annotations used for storing ILP system (otawa::ilp::System *)
 * in the CFG object.
 */
GenericIdentifier<ilp::System *> SYSTEM("ipet.system", 0);


/**
 * Identifier of annotation used for storing for storing the WCET value (int)
 * in the CFG of the computed function.
 */
GenericIdentifier<int> WCET("ipet.wcet", -1);


/**
 * Identifier of a boolean property requiring that explicit names must be used.
 * The generation of explicit names for variables may be time-consuming and
 * must only be activated for debugging purposes.
 */
GenericIdentifier<bool> EXPLICIT("ipet.explicit", false);


/**
 * Property used in the configuration of an IPET processor, it cause the
 * processor to work in a recursive way (extends its analysis to the called
 * sub-program CFGS). Default to false.
 */
GenericIdentifier<bool> RECURSIVE("ipet.recursive", false);


/**
 * Get the system tied with the given CFG. If none exists, create ones.
 * @param fw	Current framework.
 * @param cfg	Current CFG.
 * @preturn		CFG ILP system.
 */
ilp::System *getSystem(FrameWork *fw, CFG *cfg) {
	System *system = SYSTEM(cfg);
	if(!system) {
		system = fw->newILPSystem();
		if(!system)
			throw elm::MessageException("no ILP engine available");
		cfg->addDeletable<System *>(SYSTEM, system);
	}
	return system;
}


/**
 * Get the variable tied to the given basic block. If none is tied, creates a
 * new one and ties it.
 * @param system	Current ILP system.
 * @param bb		Looked basic block.
 * @return			Tied variable.
 */
ilp::Var *getVar(ilp::System *system, BasicBlock *bb) {
	Var *var = bb->get<Var *>(VAR, 0);
	if(!var) {
		var = system->newVar();
		bb->add(VAR, var);
	}
	return var;
}

/**
 * Get the variable tied to the given edge. If none is tied, creates a
 * new one and ties it.
 * @param system	Current ILP system.
 * @param edge		Looked edge.
 * @return			Tied variable.
 */
ilp::Var *getVar(ilp::System *system, Edge *edge) {
	Var *var = edge->get<Var *>(VAR, 0);
	if(!var) {
		var = system->newVar();
		edge->add(VAR, var);
	}
	return var;
}

} } // otawa::ipet
