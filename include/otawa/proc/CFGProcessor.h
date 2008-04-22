/*
 *	$Id$
 *	CFGProcessor class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-08, IRIT UPS.
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
#ifndef OTAWA_PROC_CFGPROCESSOR_H
#define OTAWA_PROC_CFGPROCESSOR_H

#include <otawa/proc/Processor.h>
#include <elm/genstruct/SLList.h>

namespace otawa {
	
// Extern class
class CFG;
class BasicBlock;
	
// Processor class
class CFGProcessor: public Registered<CFGProcessor, Processor, NullRegistration> {
	void init(const PropList& props);
protected:
	virtual void processWorkSpace(WorkSpace *fw);
	virtual void processCFG(WorkSpace *fw, CFG *cfg) = 0;
public:
	CFGProcessor(void);
	CFGProcessor(cstring name, elm::Version version);
	inline CFGProcessor(AbstractRegistration& reg): super(reg) { }
	inline CFGProcessor(cstring name, const Version& version, AbstractRegistration& reg)
		: super(name, version, reg) { }
	virtual void configure(const PropList& props);
	static void init(void);
};

// Configuration Properties
extern Identifier<CFG *> ENTRY_CFG;
extern Identifier<bool> RECURSIVE;

// Statistics Properties
extern Identifier<int> PROCESSED_CFG;

} // otawa

#endif // OTAWA_PROC_CFGPROCESSOR_H
