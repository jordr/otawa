/*
 *	$Id$
 *	display::ILPSystemDisplayer class implementation
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

#include <elm/system/System.h>
#include <otawa/ipet.h>
#include <otawa/display/ILPSystemDisplayer.h>
#include <otawa/cfg/CFGCollector.h>

namespace otawa { namespace display {

/**
 * @class ILPSystemAddon
 * This class is an interface for @ref ILPSystemDisplayer to add to its output specialized
 * information about ILP variables and constraints generators. @ref ILPSystemDisplayer
 * retrieves the list of add-ons put on the workspace using the @ref ILPSystemDisplayer::ADDON
 * identifier.
 */


/**
 * @fn cstring ILPSystemAddon::title(void) const;
 * This method is called to get the title of a specific part of the output.
 * @return	Specific part title.
 */


/**
 * @fn void ILPSystemAddon::display(io::Output& out, WorkSpace *ws, ILPSystemDisplayer *displayer);
 * This method is called when the content of the specific part need to be generated.
 * The result must be HTML plain text put to the given output stream.
 * @note If header are used, they must start at level 2.
 * @param out		Output channel to write to.
 * @param ws		Current workspace.
 * @param displayer	Current displayer.
 */


/**
 * @class ILPSystemDisplayer
 * This processor may be used to generate an HTML output of an ILP system used to compute
 * the WCET of a task.
 *
 * @par Require Features
 * @li @ref otawa::ILP_SYSTEM_FEATURE
 *
 * @par Configuration
 * @li @ref ILPSystemDisplayer::PATH
 */


/**
 */
ILPSystemDisplayer::ILPSystemDisplayer(void)
: Processor("otawa::display::ILPSystemDisplayer", Version(1, 0, 0)) {
	require(ipet::ILP_SYSTEM_FEATURE);
	require(COLLECTED_CFG_FEATURE);
}


/**
 * Configuration identifier for @ref ILPSystemDisplayer.
 * Used to select the path of the HTML file to output to.
 * As a default, the created file is TASK_NAME-ilp.html
 */
Identifier<Path> ILPSystemDisplayer::PATH("", "");


/**
 * This identifier may be used by ILP system variable and constraint generator to
 * add specific part to the output of the @ref ILPSystemDisplayer processor.
 */
Identifier<ILPSystemAddon *> ILPSystemDisplayer::ADDON(
	"otawa::display::ILPSystemDisplayer::ADDON", 0);


/**
 */
void ILPSystemDisplayer::configure(const PropList& props) {
	Processor::configure(props);
	path = PATH(props);
}


/**
 */
void ILPSystemDisplayer::processWorkSpace(WorkSpace *ws) {

	// prepare configuration
	system = ipet::SYSTEM(ws);
	if(!path)
		path = _ << ENTRY_CFG(ws)->label() << "-ilp.html";
	io::OutStream *file;
	try {
		file = system::System::createFile(path);
	}
	catch(system::SystemException& exn) {
		throw new ProcessorException(*this, _ << "cannot create file \"" << path << "\": " << exn.message());
	}
	cout.setStream(*file);

	// Build the hashtables
	typedef genstruct::Vector<ilp::Constraint *> cons_list_t;
	typedef genstruct::HashTable<ilp::Var *, cons_list_t *> vars_t;
	vars_t vars;
	for(ilp::System::ConstIterator cons(system); cons; cons++)
		for(ilp::Constraint::TermIterator term(cons); term; term++) {
			ilp::Var *var = (*term).fst;
			cons_list_t *list = vars.get(var, 0);
			if(!list) {
				list = new cons_list_t;
				vars.put(var, list);
			}
			list->add(cons);
		}

	// header
	cout << "<html>\n\t<head>\n\t\t<title>ILP system of " << ENTRY_CFG(ws)->label()
		 << " (" << ws->process()->program()->name() << ")</title>\n\t</head>\n\t<body>\n"
		 << "\t\t<h1>Content</h1>\n"
			"\t\t\t<ul>\n"
			"\t\t\t\t<li><a href=\"#__objective\">objective function</a></li>\n"
			"\t\t\t\t<li><a href=\"#__variables\">variables</a></li>\n"
			"\t\t\t\t<li><a href=\"#__constraints\">constraints</a></li>\n";
	for(Identifier<ILPSystemAddon *>::Getter addon(ws, ADDON); addon; addon++)
		cout << "\t\t\t\t<li><a href=\"#" << (void *)*addon << "\"/>" << addon->title() << "</li>\n";
	cout << "\t\t</ul>\n";

	// display objective function
	cout << "\t\t<h1><a name=\"__objective\">Objective function</a></h1>\n";
	cout << "\t\t\t<p><b>WCET</b> = " << ipet::WCET(ws) << "</p>\n"
		 << "\t\t\t<p><b>function</b><br>\n\t\t\t";
	bool first = true;
	for(ilp::System::ObjTermIterator term(system); term; term++) {
		if(first)
			first = false;
		else
			cout << " + ";
		if((*term).snd != 1)
			cout << -(*term).snd;
		displayVar((*term).fst);
	}
	cout << "\n\t\t\t</p>\n";

	// display constraints
	cout << "\t\t<h1><a name=\"__constraints\">Constraints</a></h1>\n";
	for(ilp::System::ConstIterator cons(system); cons; cons++)
		displayCons(cons);

	// display variables
	cout << "\t\t<h1><a name=\"__variables\">Variables</a></h1>\n";
	for(vars_t::PairIterator var(vars); var; var++) {
		cout << "\t\t\t<h2><a name=\"" << (*var).fst->name() << "\">" << (*var).fst->name() << " variable</a></h2>\n";
		cout << "\t\t\t\t<p>value = " << system->valueOf((*var).fst) << "</p>\n";
		ASSERT((*var).snd);
		for(int i = 0; i < (*var).snd->length(); i++)
			displayCons((*var).snd->item(i));
	}

	// display content
	for(Identifier<ILPSystemAddon *>::Getter addon(ws, ADDON); addon; addon++) {
		cout << "\t\t<h2><a name=\"" << (void *)*addon << "\">" << addon->title() << "</a></h2>\n";
		addon->display(cout, ws, this);
	}

	// tail
	cout << "\t<center><i>Generated by <a href=\"http://www.otawa.fr\">OTAWA</a> (c) 2008, IRIT - UPS</center>\n"
		 << "\t</body>\n</html>\n";
}


/**
 * Display a constraint.
 * @param cons	Constraint to display.
 */
void ILPSystemDisplayer::displayCons(ilp::Constraint *cons) {
	static cstring symbols[] = { "&lt;", "&lt;=", "=", "&gt;=", "&gt;" };
	bool first = true;
	cout << "\t\t\t<p>\n";

	// display comment
	const string& label = cons->label();
	if(label)
		cout << "\t\t\t\t<b>" << label << "</b><br/>\n";

	// before the comparator
	cout << "\t\t\t\t";
	for(ilp::Constraint::TermIterator term(cons); term; term++)
		if((*term).snd >= 0) {
			if(first)
				first = false;
			else
				cout << " + ";
			if((*term).snd != 1)
				cout << (*term).snd;
			displayVar((*term).fst);
		}

	// the comparator
	cout << ' ' << symbols[cons->comparator() - ilp::Constraint::LT] << ' ';
	if(cons->constant())
		cout << cons->constant() << io::endl;
	else
		first = true;

	// after the comparator
	for(ilp::Constraint::TermIterator term(cons); term; term++)
		if((*term).snd < 0) {
			if(first)
				first = false;
			else
				cout << " + ";
			if((*term).snd != -1)
				cout << -(*term).snd;
			displayVar((*term).fst);
		}
	cout << "\t\t\t</p>\n";
}


/**
 * Display a variable name.
 * @param var	Variable name to display.
 */
void ILPSystemDisplayer::displayVar(ilp::Var *var) {
	string name = var->name();
	cout << " <a href=\"#" << name << "\" title=\"value: " << system->valueOf(var) << "\">" << name << "</a>";
}

} } // otawa::display
