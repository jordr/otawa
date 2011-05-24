/*
 *	$Id$
 *	ClpAnalysis processor interface
 *	
 *	This file is part of OTAWA
 *	Copyright (c) 2011, IRIT UPS.
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

#ifndef OTAWA_DATA_CLP_ANALYSIS_H_
#define OTAWA_DATA_CLP_ANALYSIS_H_

#include <otawa/proc/Processor.h>
#include <otawa/data/clp/ClpState.h>

namespace otawa {

class ClpAnalysis: public Processor {
public:
	typedef Pair<const hard::Register *, Address> init_t;
	/** Initial state of the analysis */
	static Identifier<init_t> INITIAL;
	
	ClpAnalysis(void);
	virtual void configure(const PropList &props);
	
	/** @return the number of machine instructions analysed. */
	inline OCLP_STAT_UINT get_nb_inst(void){ return _nb_inst; }
	/** @return the number of semantic instructions analysed. */
	inline OCLP_STAT_UINT get_nb_sem_inst(void){ return _nb_sem_inst; }
	/** @return the number of SET analysed. */
	inline OCLP_STAT_UINT get_nb_set(void){ return _nb_set; }
	/** @return the number of SET with Top as result. */
	inline OCLP_STAT_UINT get_nb_top_set(void){ return _nb_top_set; }
	/** @return the number of STORE analysed. */
	inline OCLP_STAT_UINT get_nb_store(void){ return _nb_store; }
	/** @return the number of STORE with Top as result. */
	inline OCLP_STAT_UINT get_nb_top_store(void){ return _nb_top_store; }
	/** @return the number of STORE with Top as address. */
	inline OCLP_STAT_UINT get_nb_top_store_addr(void){return _nb_top_store_addr;}
	/** @return the number of LOAD analysed. */
	inline OCLP_STAT_UINT get_nb_load(void){return _nb_load;}
	/** @return the number of LOAD with Top as address. */
	inline OCLP_STAT_UINT get_nb_load_top_addr(void){return _nb_load_top_addr;}
	/** @return the number of filters built. */
	inline OCLP_STAT_UINT get_nb_filters(void){ return _nb_filters; }
	/** @return the number of filters with a comparison to Top. */
	inline OCLP_STAT_UINT get_nb_top_filters(void){ return _nb_top_filters; }
	
protected:
	virtual void processWorkSpace(WorkSpace *ws);
	genstruct::Vector<init_t> inits;
private:
	OCLP_STAT_UINT _nb_inst, _nb_sem_inst;
	OCLP_STAT_UINT _nb_set;
	OCLP_STAT_UINT _nb_top_set;
	OCLP_STAT_UINT _nb_store;
	OCLP_STAT_UINT _nb_top_store;
	OCLP_STAT_UINT _nb_top_store_addr;
	OCLP_STAT_UINT _nb_load;
	OCLP_STAT_UINT _nb_load_top_addr;
	OCLP_STAT_UINT _nb_filters;
	OCLP_STAT_UINT _nb_top_filters;
};

extern Feature<ClpAnalysis> CLP_ANALYSIS_FEATURE;

extern Identifier<clp::State> CLP_STATE_IN;
extern Identifier<clp::State> CLP_STATE_OUT;

}	// otawa

#endif /* OTAWA_DATA_CLP_ANALYSIS_H_ */
