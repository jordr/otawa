/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	gliss/platform.cpp -- implementation of the GLISS platform.
 */

#include <elm/debug.h>
#include <elm/io/io.h>
#define ISS_DISASM
#include "gliss.h"

namespace otawa { namespace gliss {

/**
 * @class Inst
 * Representation of instructions in Gliss.
 */

// Overloaded
address_t Inst::address(void) {
	return addr;
}

// Overloaded
size_t Inst::size(void) {
	return 4;
}

// Overloaded
Collection<Operand *> *Inst::getOps(void) {
	return 0;	// !!TODO!!
}

// Overloaded
Collection<Operand *> *Inst::getReadOps(void) {
	return 0;	// !!TODO!!
}

// Overloaded
Collection<Operand *> *Inst::getWrittenOps(void) {
	return 0;	// !!TODO!!
}

// Overloaded
void Inst::dump(io::Output& out) {
	
	// Display the hex value
	
	
	// Disassemble the statement
	code_t buffer[20];
	char out_buffer[200];
	instruction_t *inst;
	iss_fetch((::address_t)(unsigned long)addr, buffer);
	inst = iss_decode((::address_t)(unsigned long)addr, buffer);
	iss_disasm(out_buffer, inst);
	out << out_buffer;
	iss_free(inst);
}

/**
 * Scan the instruction for filling the object.
 */
void Inst::scan(void) {

	// Already computed?
	if(flags & FLAG_Built)
		return;
	
	// Get the instruction
	code_t buffer[20];
	instruction_t *inst;
	iss_fetch((::address_t)(unsigned long)address(), buffer);
	inst = iss_decode((::address_t)(unsigned long)address(), buffer);
	assert(inst);
	
	// Call customization
	scanCustom(inst);
	
	// Cleanup
	iss_free(inst);
	flags |= FLAG_Built;
}


/**
 * @class ControlInst
 * GLISS implementation of the control instruction interface.
 */

/**
 * @fn ControlInst::ControlInst(CodeSegment& segment, address_t address)
 * Build a new control instruction.
 * @param segment	Container segment.
 * @param address	Instruction address.
 */

// Overloaded	
bool ControlInst::isConditional(void) {
	scan();
	return flags & FLAG_Cond;
}

// Overloaded
otawa::Inst *ControlInst::target(void) {
	scan();
	return _target;
}

// Overload
void ControlInst::scanCustom(instruction_t *inst) {

	// Explore the instruction
	address_t target_addr = 0;
	switch(inst->ident) {
	case ID_BL_:
		flags |= FLAG_Call;
	case ID_B_:
		assert(inst->instrinput[0].type == PARAM_INT24_T);
		target_addr = address() + (inst->instrinput[0].val.Int24 << 2);
		break;
	case ID_BLA_:
		flags |= FLAG_Call;
	case ID_BA_:
		assert(inst->instrinput[0].type == PARAM_INT24_T);
		target_addr = (address_t)(inst->instrinput[0].val.Int24 << 2);
		break;
	case ID_BCL_:
		flags |= FLAG_Call;
	case ID_BC_:
		flags |= FLAG_Cond;
		assert(inst->instrinput[2].type == PARAM_INT14_T);
		target_addr = address() + (inst->instrinput[2].val.Int14 << 2);
		break;
	case ID_BCLA_:
		flags |= FLAG_Call;
	case ID_BCA_:
		flags |= FLAG_Cond;
		assert(inst->instrinput[2].type == PARAM_INT14_T);
		target_addr = (address_t)(inst->instrinput[2].val.Int14 << 2);
		break;
	case ID_SC:
		flags |= FLAG_Call;
		break;
	case ID_BCLR_:
		assert(inst->instrinput[0].type == PARAM_UINT5_T);
		assert(inst->instrinput[1].type == PARAM_UINT5_T);
		if(inst->instrinput[0].val.Uint5 == 20
		&& inst->instrinput[1].val.Uint5 == 0) {
			flags |= FLAG_Return;
			break;
		}
	case ID_BCLRL_:
		flags |= (FLAG_Return | FLAG_Cond);
		break;
	case ID_BCCTRL_:
		flags |= FLAG_Call;
	case ID_BCCTR_:
	cond:
		flags |= FLAG_Cond;
		break;
	}
	
	// Compute the target if address available
	if(target_addr)
		_target = (Inst *)segment().findByAddress(target_addr);
}

// Overloaded
bool ControlInst::isControl(void) {
	return true;
}

// Overloaded
bool ControlInst::isBranch(void) {
	return !(flags & (FLAG_Call | FLAG_Return));
}

// Overloaded
bool ControlInst::isCall(void) {
	return flags & FLAG_Call;
}

// Overloaded
bool ControlInst::isReturn(void) {
	return flags & FLAG_Return;
}

} }	// otawa::gliss
