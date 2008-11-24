/*
 *	$Id$
 *	gensim module interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007-08, IRIT UPS.
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef OTAWA_GENSIM_GENERIC_STATE_H
#define OTAWA_GENSIM_GENERIC_STATE_H

#include "GenericProcessor.h"
#include "SimulationStats.h"
#include "Memory.h"
#include <otawa/sim/Simulator.h>
#include <otawa/sim/State.h>
#include <otawa/otawa.h>
#include <otawa/prop/Identifier.h>
#include <otawa/sim/AbstractCacheDriver.h>
#include "MemoryDriver.h"

namespace otawa {
namespace gensim {

class SimulationStats;
class GenericProcessor;
class MemoryStats;

//extern Identifier<int> INSTRUCTION_TIME; ---- A REMETTRE

// extern Identifier<String&> TRACE_FILE;

extern  Identifier<sim::CacheDriver*> ICACHE;
extern  Identifier<sim::CacheDriver*> DCACHE;



class GenericState : public sim::State {
	friend class GenericSimulator;
	//friend class MemoryStats;
	WorkSpace *fw;
	int _cycle;
	GenericProcessor* processor;
	bool running;
	void step(void);
	elm::genstruct::SLList<SimulationStats *> stats;
public:
	sim::Driver *driver;
	sim::CacheDriver *icache_driver;
	sim::CacheDriver *dcache_driver;
	sim::MemoryDriver *mem_driver;
	GenericState(WorkSpace *framework) :
		fw(framework), _cycle(0), driver(NULL) {
	}
	GenericState(const GenericState& state) :
		fw(state.fw), _cycle(state._cycle), driver(NULL) {
	}
	void init();
	inline void addStats(SimulationStats * new_stats) {
		stats.addLast(new_stats);
	}
	void dumpStats(elm::io::Output& out_stream) {
		for (elm::genstruct::SLList<SimulationStats *>::Iterator stat(stats); stat; stat++) { 
			stat->dump(out_stream);
		}
		out_stream << "\n";
	}
	
	//functions to recover stats from the memory system
/* 	int getNumberOfInstructionAccesses(); */
/* 	int getNumberOfInstructionCacheHits(); */
/* 	int getNumberOfInstructionCacheMisses(); */
/* 	int getNumberOfDataAccesses(); */
/* 	int getNumberOfDataReads(); */
/* 	int getNumberOfDataWrites(); */
/* 	int getNumberOfDataCacheHits(); */
/* 	int getNumberOfDataCacheMisses(); */

	void resetStats(void) {
		for (elm::genstruct::SLList<SimulationStats *>::Iterator stat(stats); stat; stat++)
			stat->reset();
	}

	// State overload
	virtual State *clone(void) {
		return new GenericState(*this);
	}
	virtual void run(sim::Driver& driver) {
		this->driver = &driver;
		if(!icache_driver)
			this->icache_driver = &(sim::CacheDriver::ALWAYS_HIT);
		running = true;
		while (running)
			step();
	}
	virtual void run(sim::Driver& driver, sim::CacheDriver* icache_driver,
			sim::CacheDriver* dcache_driver, sim::MemoryDriver *mem_driver) {
		this->driver = &driver;
		if (!icache_driver) {
		  if (ICACHE(this))
		    this->icache_driver = ICACHE(this);
		  else
		    this->icache_driver = &(sim::CacheDriver::ALWAYS_MISS);
		}
		else
		  this->icache_driver = icache_driver;
		if (!dcache_driver){
		  if (DCACHE(this))
		    this->dcache_driver = DCACHE(this);
		  else
		    this->dcache_driver = &(sim::CacheDriver::ALWAYS_MISS);
		}
		else
			this->dcache_driver = dcache_driver;
		if (!mem_driver)
			this->mem_driver = &(sim::MemoryDriver::ALWAYS_DATA_CACHE);
		else
			this->mem_driver = mem_driver;
		running = true;
		while (running)
			step();
	}
	virtual void stop(void) {
		running = false;
	}
	virtual void flush(void) {
	}
	virtual int cycle(void) {
		return _cycle;
	}
	virtual void reset(void) {
		_cycle = 0;
	}
	void resetProc(void);
};

}
} // otawa::gensim


#endif // OTAWA_GENSIM_GENERIC_STATE_H
