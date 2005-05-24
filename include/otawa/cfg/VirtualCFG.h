/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/cfg/VirtualCFG.h -- interface of VirtualCFG class.
 */
#ifndef OTAWA_CFG_VIRTUAL_CFG_H
#define OTAWA_CFG_VIRTUAL_CFG_H

#include <otawa/cfg/CFG.h>

namespace otawa {
	
// VirtualCFG class
class VirtualCFG: public CFG {
	CFG *_cfg;
	void virtualize(struct call_t *stack, CFG *cfg,
		BasicBlock *entry, BasicBlock *exit);
protected:
	virtual void scan(void);
public:
	static Identifier ID_CalledCFG;
	static Identifier ID_Recursive;
	VirtualCFG(CFG *cfg, bool inlined = true);
	inline CFG *cfg(void) const;
};

// Inlines
inline CFG *VirtualCFG::cfg(void) const {
	return _cfg;
}

} // otawa

#endif	// OTAWA_CFG_VIRTUAL_CFG_H
