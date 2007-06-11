#include <stdio.h>
#include <elm/io.h>
#include <elm/genstruct/Vector.h>
#include <elm/genstruct/Table.h> 
#include <otawa/cache/LBlockSet.h>
#include <otawa/util/LBlockBuilder.h>
#include <otawa/ilp.h>
#include <otawa/ipet.h>
#include <otawa/util/Dominance.h>

#include <otawa/cfg.h>
#include <otawa/util/LoopInfoBuilder.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/hard/Platform.h>


#include <otawa/cache/FirstLastBuilder.h>

using namespace otawa;
using namespace otawa::ilp;
using namespace otawa::ipet;
using namespace elm;




namespace otawa {

/**
 * 
 * This feature represents the availability of the LAST_LBLOCK and LBLOCK_ISFIRST properties.
 * 
 * @par Properties
 * @li @ref LAST_LBLOCK
 * @li @ref LBLOCK_ISFIRST
 */
Feature<FirstLastBuilder> ICACHE_FIRSTLAST_FEATURE("otawa.cache.firstlast_feature");

 
/**
 * This property gives the ID of the last lblock of a BasicBlock, for each line.
 *
 * @par Hooks
 * @li @ref BasicBlock 
 */
Identifier<LBlock**> LAST_LBLOCK("otawa.cache.last_lblock", NULL, otawa::NS);
 
 /**
 * This property tells if the lblock is the first of its BasicBlock (for its cache line)
 * This information is useful, because if it's false, then, the lblock is always a miss.
 *
 * @par Hooks
 * @li @ref LBlock 
 */
Identifier<bool> LBLOCK_ISFIRST("otawa.cache.lblock_isfirst", false, otawa::NS);
 
/**
 * @class ACSBuilder
 *
 * This processor produces the Abstract Cache States (ACS), for the May and Persistence problems.
 *
 * @par Configuration
 * none
 *
 * @par Required features
 * @li @ref DOMINANCE_FEATURE
 * @li @ref LOOP_HEADERS_FEATURE
 * @li @ref LOOP_INFO_FEATURE
 * @li @ref COLLECTED_LBLOCKS_FEATURE
 *
 * @par Provided features
 * @li @ref ICACHE_FIRSTLAST_FEATURE
 * 
 * @par Statistics
 * none
 */

FirstLastBuilder::FirstLastBuilder(void) : CFGProcessor("otawa::FirstLastBuilder", Version(1, 0, 0)) {
	require(DOMINANCE_FEATURE);
	require(LOOP_HEADERS_FEATURE);
	require(LOOP_INFO_FEATURE);
	require(COLLECTED_LBLOCKS_FEATURE);
	provide(ICACHE_FIRSTLAST_FEATURE);
}

void FirstLastBuilder::processCFG(WorkSpace *fw, CFG *cfg) {
	int i;
	address_t *max_addr, *min_addr;
	LBlock **min_lblock, **max_lblock;

		
	LBlockSet **lbsets = LBLOCKS(fw);
	const hard::Cache *cache = fw->platform()->cache().instCache();
		
	for (CFG::BBIterator bb(cfg); bb; bb++)
			LAST_LBLOCK(bb) = new LBlock*[cache->lineCount()];
	
	max_addr = new address_t[cache->lineCount()];
	min_addr = new address_t[cache->lineCount()];
	max_lblock = new LBlock*[cache->lineCount()];
	min_lblock = new LBlock*[cache->lineCount()];
	
	/*
	 * Detects the first/last lblock for each basicblock (for the current cache line)
	 * This information is used for the ACS creation
	 */
	for (CFG::BBIterator bb(cfg); bb; bb++) {
		for (int line = 0; line < cache->lineCount(); line++) {
			max_lblock[line] = NULL;
			min_lblock[line] = NULL;
		}			
		if (BB_LBLOCKS(bb) != NULL) { 
			const genstruct::AllocatedTable<LBlock*> &lblocks = *BB_LBLOCKS(bb);
		
			for (i = 0; i < lblocks.count(); i++) { 
				int line = cache->line(lblocks[i]->address());
				
				
				if ((max_lblock[line] == NULL) || (lblocks[i]->address() > max_addr[line])) {
					max_addr[line] = lblocks[i]->address();
					max_lblock[line] = lblocks[i];
				}
				if ((min_lblock[line] == NULL) || (lblocks[i]->address() < min_addr[line])) {
					min_addr[line] = lblocks[i]->address();
					min_lblock[line] = lblocks[i];
				}
			}
		}
		for (int line = 0; line < cache->lineCount(); line++) {
			/* XXX TODO: pas coh�rent */
			LAST_LBLOCK(bb)[line] = max_lblock[line];
			if (min_lblock[line] != NULL)
				LBLOCK_ISFIRST(min_lblock[line]) = true;
		}
	}
	
	delete [] max_addr;	
	delete [] min_addr;
	delete [] max_lblock;
	delete [] min_lblock;	
}

}
