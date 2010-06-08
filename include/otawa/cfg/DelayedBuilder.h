/*
 *	$Id$
 *	DelayedBuilder
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2008, IRIT UPS.
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
#ifndef OTAWA_DELAYEDBUILDER_H_
#define OTAWA_DELAYEDBUILDER_H_

#include <elm/genstruct/SLList.h>
#include <otawa/proc/CFGProcessor.h>
#include <otawa/cfg/features.h>

namespace otawa {

class Inst;
class Process;
class VirtualCFG;

class DelayedBuilder: public CFGProcessor {
public:
	static Registration<DelayedBuilder> reg;
	DelayedBuilder(void);

protected:
	virtual void setup(WorkSpace *ws);
	virtual void processWorkSpace(WorkSpace *ws);
	virtual void processCFG(WorkSpace *ws, CFG *cfg);
	virtual void cleanup(WorkSpace *ws);

private:
	class Cleaner: public otawa::Cleaner {
	public:
		inline Cleaner(Process *process): proc(process) { }
		virtual ~Cleaner(void);
		inline void addNOP(Inst *inst) { nops.add(inst); }
	private:
		Process *proc;
		genstruct::SLList<Inst *> nops;
	};

	CFGCollection *coll;
	Cleaner *cleaner;
	genstruct::HashTable<CFG *, VirtualCFG *> cfg_map;
};

} // otawa


#endif /* OTAWA_DELAYEDBUILDER_H_ */
