/*
 *	$Id$
 *	OTAWA Instruction Disassembler
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2010, IRIT UPS.
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

#include <otawa/app/Application.h>
#include <otawa/prog/TextDecoder.h>
#include <otawa/hard/Register.h>
#include <elm/genstruct/SortedSLList.h>
#include <elm/option/SwitchOption.h>
#include <otawa/cfg/features.h>
#include <elm/genstruct/AVLTree.h>
#include <otawa/prog/sem.h>

using namespace elm;
using namespace otawa;

static struct {
	Inst::kind_t kind;
	cstring name;
} kinds[] = {
	{ Inst::IS_COND, "COND" },
	{ Inst::IS_CONTROL, "CONTROL" },
	{ Inst::IS_CALL, "CALL" },
	{ Inst::IS_RETURN, "RETURN" },
	{ Inst::IS_MEM, "MEM" },
	{ Inst::IS_LOAD, "LOAD" },
	{ Inst::IS_STORE, "STORE" },
	{ Inst::IS_INT, "INT" },
	{ Inst::IS_FLOAT, "FLOAT" },
	{ Inst::IS_ALU, "ALU" },
	{ Inst::IS_MUL, "MUL" },
	{ Inst::IS_DIV, "DIV" },
	{ Inst::IS_SHIFT, "SHIFT" },
	{ Inst::IS_TRAP, "TRAP" },
	{ Inst::IS_INTERN, "INTERN" },
	{ Inst::IS_MULTI, "MULTI" },
	{ Inst::IS_SPECIAL, "SPECIAL" },
	{ 0, "" }
};


class ODisasm: public Application {
public:

	ODisasm(void)
	: Application(
		"odisasm",
		Version(1, 0, 0),
		"Disassemble instruction to OTAWA instruction description",
		"H. Cassé <casse@irit.fr>"),
	regs(*this, option::cmd, "-r", option::cmd, "--regs", option::help, "display register information", option::end),
	kind(*this, option::cmd, "-k", option::cmd, "--kind", option::help, "display kind of instructions", option::end),
	sem(*this, option::cmd, "-s", option::cmd, "--semantics", option::help, "display translation of instruction in semantics language", option::end),
	target(*this, option::cmd, "-t", option::cmd, "--target", option::help, "display target of control instructions", option::end)
	{ }

	virtual void work (const string &entry, PropList &props) throw (elm::Exception) {
		require(otawa::COLLECTED_CFG_FEATURE);
		const CFGCollection *coll = otawa::INVOLVED_CFGS(workspace());
		for(int i = 0; i < coll->count(); i++)
			processCFG(coll->get(i));

	}


private:

	class BasicBlockComparator {
	public:
		static int compare(const BasicBlock *v1, const BasicBlock *v2) { return v1->address() - v2->address(); }
	};

	/**
	 * Disassemble a CFG.
	 */
	void processCFG(CFG *cfg) {
		cout << "# FUNCTION " << cfg->label() << io::endl;

		// put BB in the right order
		typedef elm::genstruct::AVLTree<BasicBlock *, Id<BasicBlock *>, BasicBlockComparator> avl_t;
		avl_t avl;
		for(CFG::BBIterator bb(cfg); bb; bb++)
			if(!bb->isEnd())
				avl.add(bb);

		// disassemble the BB
		for(avl_t::Iterator bb(avl); bb; bb++)
			for(BasicBlock::InstIter inst(bb); inst; inst++)
				processInst(inst);
	}

	/**
	 * Disassemble an instruction.
	 * @param	inst	Instruction to disassemble.
	 */
	void processInst(Inst *inst) {

		// disassemble labels
		for(Identifier<String>::Getter label(inst, FUNCTION_LABEL); label; label++)
			cout << '\t' << *label << ":\n";
		for(Identifier<String>::Getter label(inst, LABEL); label; label++)
			cout << '\t' << *label << ":\n";

		// disassemble instruction
		cout << inst->address() << "\t" << inst << io::endl;

		// display kind if required
		if(kind) {
			cout << "\tkind = ";
			Inst::kind_t kind = inst->kind();
			for(int i = 0; kinds[i].kind; i++)
				if(kinds[i].kind & kind)
					cout << kinds[i].name << ' ';
			cout << io::endl;
		}

		// display target if any
		if(target) {
			if(inst->isControl() && !inst->isReturn() && !(kind & Inst::IS_TRAP)) {
				cout << "\ttarget = ";
				Inst *target = inst->target();
				if(!target)
					cout << "unknown";
				cout << io::endl;
			}
		}

		// display registers
		if(regs) {

			// display read registers
			genstruct::SortedSLList<string> srr;
			const elm::genstruct::Table<hard::Register * >& rr = inst->readRegs();
			for(int i = 0; i < rr.count(); i++)
				srr.add(rr[i]->name());
			cout << "\tread regs = ";
			for(genstruct::SortedSLList<string>::Iterator r(srr); r; r++)
				cout << *r << " ";
			cout << io::endl;

			// display read registers
			genstruct::SortedSLList<string> swr;
			const elm::genstruct::Table<hard::Register * >& wr = inst->writtenRegs();
			for(int i = 0; i < wr.count(); i++)
				swr.add(wr[i]->name());
			cout << "\twritten regs = ";
			for(genstruct::SortedSLList<string>::Iterator r(swr); r; r++)
				cout << *r << " ";
			cout << io::endl;
		}

		// display semantics
		if(sem) {
			otawa::sem::Block block;
			inst->semInsts(block);
			cout << "\t\tsemantics\n";
			for(int i = 0; i < block.count(); i++)
				cout << "\t\t\t" << block[i] << io::endl;
		}
	}

	option::SwitchOption regs, kind, sem, target;
};

int main(int argc, char **argv) {
	ODisasm app;
	return app.run(argc, argv);
}
