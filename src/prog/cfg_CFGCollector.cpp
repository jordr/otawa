/*
 *	$Id$
 *	Copyright (c) 2006 IRIT - UPS.
 *
 *	prog/cfg_CollectCFG.h -- implementation of CFGCollector class.
 */

#include <elm/assert.h>
#include <otawa/cfg/CFGCollector.h>
#include <otawa/cfg.h>
#include <otawa/cfg/CFGBuilder.h>
#include <otawa/proc/CFGProcessor.h>
#include <otawa/prog/Manager.h>

namespace otawa {

static Identifier<bool> MARK("otawa.cfg_collector.mark", false);

/**
 * @class CFGCollection <otawa/cfg.h>
 * Contains a collection of CFGs (used with @ref INVOLVED_CFGS property).
 *
 * @par Configuration
 * @li @ref ENTRY_CFG: CFG of the entry of the current task,
 * @li @ref TASK_ENTRY: name if the entry function of the current task,
 * @li @ref RECURSIVE: collect CFG recursively.
 * @li @ref CFGCollector::ADDED_CFG: CFG to add to the collection.
 * @li @ref CFGCollecyor::ADDED_FUNCTION: function name to add to the collection.
 * 
 * @par Provided Features
 * @ref COLLECTED_CFG_FEATURE
 * 
 * @par Required Features
 * @li @ref CFG_INFO_FEATURE
 */


/**
 * @fn int CFGCollection::count(void) const;
 * Get the count of CFG in the collection.
 * @return CFG count.
 */


/**
 * @fn CFG *CFGCollection::get(int index) const;
 * Get a CFG from the collection using its index.
 * @param index	Index of the got CFG.
 * @return		CFG matching the index.
 */


/**
 * @fn CFG *CFGCollection::operator[](int index) const;
 * Shortcut to @ref get().
 */


/**
 * @class CFGCollection::Iterator
 * Iterator on the CFG contained in a @ref CFGCollection.
 */


/**
 * @fn CFGCollection::Iterator::Iterator(const CFGCollection *cfgs);
 * Build an iterator on the given CFG collection.
 * @param cfgs	CFG collection to iterate on.
 */


/**
 * @fn CFGCollection::Iterator(const CFGCollection& cfgs);
 * Build an iterator on the given CFG collection.
 * @param cfgs	CFG collection to iterate on.
 */


/**
 * @class CFGCollector
 * This processor is used to collect all CFG implied in a computation.
 * It uses the @ref ENTRY_CFG or @ref TASK_ENTRY properties to find
 * the base CFG and explore in depth the CFG along subprograms calls.
 */


/**
 */
void CFGCollector::processWorkSpace (WorkSpace *fw) {

        int index = 0;
	
	// Set first queue node
	if(!entry && name) {
		CFGInfo *info = fw->getCFGInfo();
		CString name = TASK_ENTRY(fw);
		entry = info->findCFG(name);
	}
	if(!entry)
		throw ProcessorException(*this, _
			<< "cannot find task entry point \"" << name << "\"");
	ENTRY_CFG(fw) = entry;
	
	// Build the involved collection
	CFGCollection *cfgs = new CFGCollection();
	INDEX(entry) = index;
	index++;
	
	// Entry CFG
	cfgs->cfgs.add(entry);
	
	// Added functions
	for(int i = 0; i < added_funs.length(); i++) {
		CFGInfo *info = fw->getCFGInfo();
		CFG *cfg = info->findCFG(added_funs[i]);
		if(cfg)
			cfgs->cfgs.add(cfg);
		else
			warn(_ << "cannot find a function called \"" << added_funs[i] << "\".");
	}
	
	// Added CFG
	for(int i = 0; i < added_cfgs.length(); i++)
		cfgs->cfgs.add(added_cfgs[i]);

	INVOLVED_CFGS(fw) = cfgs;
	
	// Build it recursively
	if(rec)
		for(int i = 0; i < cfgs->count(); i++)
			for(CFG::BBIterator bb(cfgs->get(i)); bb; bb++)
				for(BasicBlock::OutIterator edge(bb); edge; edge++)
					if(edge->kind() == Edge::CALL
					&& edge->calledCFG()
					&& !MARK(edge->calledCFG())) {
					        INDEX(edge->calledCFG()) = index;
					        index++;
						cfgs->cfgs.add(edge->calledCFG());
						MARK(edge->calledCFG()) = true;
					} 
}


/**
 * Build the CFG collector.
 * @param props	Configuration properties.
 */
CFGCollector::CFGCollector(void)
: Processor("CFGCollector", Version(1, 0, 0)), entry(0), rec(false) {
	require(CFG_INFO_FEATURE);
	provide(COLLECTED_CFG_FEATURE);
}


/**
 */
void CFGCollector::configure(const PropList& props) {
	Processor::configure(props);
	
	// Misc configuration
	entry = ENTRY_CFG(props);
	if(!entry)
		name = TASK_ENTRY(props);
	rec = otawa::RECURSIVE(props);

	// Collect added CFGs
	added_cfgs.clear();
	for(Identifier<CFG *>::Getter cfg(props, ADDED_CFG); cfg; cfg++)
		added_cfgs.add(cfg);
	added_funs.clear();
	for(Identifier<CString>::Getter fun(props, ADDED_FUNCTION); fun; fun++)
		added_funs.add(fun);
}


/**
 * This property is used to link the current computation involved CFG
 * on the framework.
 * 
 * @par Hooks
 * FrameWork
 */
Identifier<CFGCollection *> INVOLVED_CFGS("involved_cfgs", 0, otawa::NS);


/**
 * This feature asserts that all CFG involved in the current computation has
 * been collected and accessible thanks to @ref INVOLVED_CFGS property
 * 
 * @par Properties
 * @ref ENTRY_CFG (FrameWork).
 * @ref INVOLVED_CFGS (FrameWork).
 */
Feature<CFGCollector> COLLECTED_CFG_FEATURE("otawa::collected_cfg");


/**
 * This configuration property allows to add unlinked CFG to the used CFG
 * collection.
 */
Identifier<CFG *> CFGCollector::ADDED_CFG("CFGCollector::ADDED_CFG", 0, otawa::NS);


/**
 * This configuration property allows to add unlinked functions to the used CFG
 * collection.
 */
Identifier<CString> CFGCollector::ADDED_FUNCTION("CFGCollector::ADDED_FUNCTION", 0, otawa::NS);

} // otawa
