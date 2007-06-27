/*
 *	$Id$
 *	This is the Abstract Cache State builder.
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007, IRIT UPS.
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */
#ifndef CACHE_ACSBUILDER_H_
#define CACHE_ACSBUILDER_H_

#include <otawa/prop/Identifier.h>
#include <otawa/proc/Processor.h>
#include <otawa/hard/Cache.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/cache/cat2/MUSTProblem.h>
#include <otawa/cache/cat2/PERSProblem.h>

namespace otawa {
	
using namespace elm;


typedef enum fmlevel_t {
		FML_INNER = 0,
		FML_OUTER = 1,
		FML_MULTI = 2,
		FML_NONE
} fmlevel_t;
	
extern Identifier<genstruct::Vector<MUSTProblem::Domain*>* > CACHE_ACS_MUST;
extern Identifier<genstruct::Vector<PERSProblem::Domain*>* > CACHE_ACS_PERS;
extern Identifier<fmlevel_t> FIRSTMISS_LEVEL;
extern Identifier<bool> PSEUDO_UNROLLING;
extern Identifier<MUSTProblem::Domain*> CACHE_ACS_MUST_ENTRY;

class ACSBuilder : public otawa::Processor {

	void processLBlockSet(otawa::WorkSpace*, LBlockSet *, const hard::Cache *);	
	fmlevel_t level;
	
	bool unrolling;
	MUSTProblem::Domain *must_entry;
	
	public:
	ACSBuilder(void);
	virtual void processWorkSpace(otawa::WorkSpace*);
	virtual void configure(const PropList &props);
	
	
	
};

extern Feature<ACSBuilder> ICACHE_ACS_FEATURE;

}

#endif /*CACHE_ACSBUILDER_H_*/
