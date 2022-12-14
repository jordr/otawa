	/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/include/otawa/util/DFA.h -- DFA class implementation.
 */

#include <elm/assert.h>
#include <otawa/cat/CATDFA.h>
#include <otawa/cfg.h>
#include <otawa/instruction.h>
#include <otawa/cat/CATConstraintBuilder.h>
#include <otawa/cache/LBlockSet.h>
#include <elm/data/HashMap.h>
#include <otawa/cat/CATBuilder.h>

using namespace otawa::ilp;
using namespace otawa::ipet;

namespace otawa { namespace cat {

/**
 */
int CATProblem::vars = 0;


/**
 */
CATDomain *CATProblem::gen(CFG *cfg, Block *bb) {
	int length = lines->count();
	CATDomain *bitset = new CATDomain(length);
	if (bb->isEntry()){
		bitset->add(0);
		return bitset;
	}
	else if(bb->isBasic()) {
		address_t adlbloc;
	    int identif=0;	    
	    for(BasicBlock::InstIter inst = bb->toBasic()->insts(); inst; inst++) {
			adlbloc = inst->address();				
			for (cache::LBlockSet::Iterator lbloc(*lines); lbloc; lbloc++){
				address_t address = lbloc->address();
				if ((adlbloc == address)&& (bb == lbloc->bb()))
					identif = lbloc->id();
				}
	    }// end for
	
	    if (identif != 0)bitset->add(identif);
	    	return bitset;
	}
	else
		return bitset;
}

/**
 */
CATDomain *CATProblem::preserve(CFG *cfg, Block *bb) {
	//Inst *inst;
	if(!bb->isBasic())
		return new CATDomain(lines->count());
	bool testnotconflit = false;
	bool visit = false;
	address_t adlbloc;
	int identif1 = 0 , identnonconf = 0 , identif2 = 0;
	for(BasicBlock::InstIter inst = bb->toBasic()->insts(); inst; inst++) {
		visit = false;
		// decallage x where each block containts 2^x ocets
		int dec = cach->blockBits();
		adlbloc = inst->address();
		if (!testnotconflit){
			// lblocks iteration
			for (cache::LBlockSet::Iterator lbloc(*lines); lbloc; lbloc++){
				if ((adlbloc == (lbloc->address()))&&(bb == (lbloc->bb()))){
					//testnotconflit = true;
					identif1 = lbloc->id();
					unsigned long tag = ((unsigned long)adlbloc.offset()) >> dec;
					for (cache::LBlockSet::Iterator lbloc1(*lines); lbloc1; lbloc1++){
						unsigned long taglblock = ((unsigned long)lbloc1->address().offset()) >> dec;
						if (adlbloc != (lbloc1->address())&&(tag == taglblock)){
							identnonconf = lbloc1->id();
							// the state of the first lblock in BB become nonconflict
							cache::LBlock *ccgnode = lines->lblock(identif1);
							//								ccgnode->setNonConflictState(true);
							cat::CATBuilder::NON_CONFLICT(ccgnode) = true;
							break;
						}// end Sde if
					}//end Sde for of lbloc
					break;
				}//end first if

			}// end first for of lblock
			testnotconflit = true;
			visit = true;
		}//end first if of testnonconf


		if (!visit){
			for (cache::LBlockSet::Iterator lbloc(*lines); lbloc; lbloc++){
				if (adlbloc == (lbloc->address())){
					identif2 = lbloc->id();
					break ;
				}// end Sde if
			}// end Sde for
		}//end first if

	}// end for
	// the bit vector of kill
	int length = lines->count();

	CATDomain *kill;
	kill = new CATDomain(length);

	if (!((identif1 == 0) && (identif2 == 0))) {
		kill->fill();
		if ((identif1 != 0)&&(identif2 == 0)&&(identnonconf != 0))
			kill->remove(identnonconf);
	}
	kill->complement();
	return kill ;
}



} } // otawa::cat
