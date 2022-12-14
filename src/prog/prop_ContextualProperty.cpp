/*
 * OTAWA -- WCET computation framework
 * Copyright (C) 2009  IRIT - UPS <casse@irit.fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.package ojawa;
 */

#include <elm/data/Vector.h>
#include <otawa/prop/ContextualProperty.h>
#include <otawa/prop/AbstractIdentifier.h>

namespace otawa {

/**
 * @defgroup context Contextual Information
 *
 * OTAWA allows to define properties on a contextual base. A contextual path (@ref ContextualPath)
 * defines a situation when a property value applies. For example, a loop bound may depend on
 * the call chain leading to the loop.
 *
 * @p
 *
 * Contextual paths are more general than simple call chains. They may includes:
 * @li @ref ContextualStep::FUNCTION -- function call (address of the function as parameter),
 * @li @ref ContextualStep::CALL -- function call site (address of the call as parameter),
 * @li @ref ContextualStep::FIRST_ITER -- first iteration of a loop,
 * @li @ref ContextualStep::OTHER_ITER -- other iterations of a loop,
 * @li @ref ContextualStep::AFTER -- after executing a BB (which address is passed as parameter).
 *
 *
 * @section context_get Getting a contextual property
 *
 * Getting a property is almost as simple as getting a normal property.
 * We first need to build a contextual path [c1, c2, ..., cn] with cn being the closest component
 * of the property list receiving the contextual property. For example, c1 may be the main()
 * function of the program to time while cn is the function containing the denoted item.
 * In another example, the contextual path [FUNCTION "main", FUNCTION "f", FUNCTION "g"] applied on
 * basic block B means that the property applies when "main" calls "f" that calls "g" that contains
 * the basic block B.
 *
 * With the last example, getting the property value (loop bound here) becomes:
 * @code
 *	#include <otawa/prop/ContextualProperty.h>
 *
 *	BasicBlock *bb;
 *
 *	ContextualPath path;
 *	path.push(ContextualStep::FUNCTION, main_cfg->address());
 *	path.push(ContextualStep::FUNCTION, f_cfg->address());
 *	path.push(ContextualStep::FUNCTION, g_cfg->address());
 *	int loop_bound = path(MAX_ITERATION, bb);
 * @endcode
 *
 * Usually, contextual paths are not build explicitly as in the example above.
 * They are obtained from algorithm traversing the program structure representation
 * like @ref otawa::ContextualProcessor .
 *
 * Notice also that the contextual path may match imprecisely defined property.
 * For example, the property defined at the contextual path [FUNCTION "f", FUNCTION "g"]
 * may be obtained by the contextual path [FUNCTION "f", CALL 0xffff0010, FUNCTION "g"]
 * as the second path is just a special case compared to the more general one of the
 * first path.
 *
 *
 * @section context_set Setting a contextual property
 *
 * First, notice that you do not need to give the full path. For example, the path
 * [FUNCTION "f", FUNCTION "g"] means that the property applies each time "g" is called
 * from "f" whatever the previous call chain is.
 *
 */

/**
 * @class ContextualStep
 * A context is a path representing different entities composing a program
 * and a step is the smallest part of this path. A contextual step is defined
 * by a kind (function, call, loop, etc) and an address allowing to identify
 * uniquely the entity.
 *
 * @ingroup context
 */


/**
 */
void ContextualStep::print(io::Output& out) const {
	switch(_kind) {
	case NONE: out << "NONE"; break;
	case FUNCTION: out << "FUN(" << addr << ")"; break;
	case CALL: out << "CALL(" << addr << ")"; break;
	case FIRST_ITER: out << "FIRST_ITER(" << addr << ")"; break;
	case OTHER_ITER: out << "OTHER_ITER(" << addr << ")"; break;
	case AFTER: out << "AFTER(" << addr << ")"; break;
	}
}


/**
 */
static bool print_rec(io::Output& out, ContextualList *l) {
	if(!l)
		return false;
	else {
		if(print_rec(out, &(l->next())))
			out << ", ";
		out << l->step();
		return true;
	}
}


/**
 * Print the contextual path.
 * @param out	Stream to output to.
 */
void ContextualPath::print(io::Output& out) const {
	out << "[";
	print_rec(out, &p);
	out << "]";
}


/**
 * Get the address of the function enclosing immediately the current context.
 * @return	Enclosing function address or Address::null if no function is found.
 */
Address ContextualPath::getEnclosingFunction(void) const {
	for(ContextualList *l = &p; l; l = &(l->next()))
		if(p->step().kind() == ContextualStep::FUNCTION)
			return p->step().address();
	return Address::null;
}


/**
 * @class ContextualProperty
 * A contextual property is a special property that allows to retrieve
 * a property value by its contextual path (@ref ContextualPath).
 * Contextual path allows to takes into account the context of execution of
 * a block, that is, the chaining of function, calls and loop iteration
 * driving to the block.
 * @p
 * Notice that the path must be precise while the matching is
 * performed on a blurred contextual tree. This means that
 * some parts of the contextual path may be ignored if an
 * embedding context is found in the contextual property.
 * For example, the path [c1, c2, c3] may match a property
 * whose path is [c1, c3] as it is considered as [c1, *, c2],
 * that is more general that the given path. * means "any
 * component".
 *
 * @ingroup context
 */


/**
 * Build a contextual property.
 */
ContextualProperty::ContextualProperty(void): Property(ID) {
}


/**
 * Find the property list matching the best the given path.
 * @param path	Path to match.
 * @param stack	Stack of property list leading to the full context.
 * @return		More precise property list matching the path.
 */
const PropList& ContextualProperty::findProps(
	const PropList& props,
	const ContextualPath& path,
	const AbstractIdentifier& id
) const {
	Vector<const PropList *> stack;

	// fill the stack
	stack.push(&props);
	const Node *node = &root;
	for(const ContextualList *l = path.list(); l; l = &l->next())
		for(inhstruct::Tree::Iter child(node); child; child++) {
			Node *cur = (Node *)*child;
			if(cur->step == l->step()) {
				stack.push(cur);
				node = cur;
				break;
			}
		}

	// find the identifier
	for(int i = stack.count() - 1; i >= 0; i--)
		if(stack[i]->hasProp(id))
			return *stack[i];
	return props;
}


/**
 * Find or make the property list for the given path.
 * @param path	Contextual path.
 * @return		Property matching the given path.
 */
PropList& ContextualProperty::makeProps(const ContextualPath& path) {
	Node *parent = &root;
	for(const ContextualList *l = path.list(); l; l = &l->next()) {
		bool found = false;
		for(inhstruct::Tree::Iter child(parent); child; child++) {
			Node *node = (Node *)*child;
			if(node->step == l->step()) {
				parent = node;
				found = true;
				break;
			}
		}
		if(!found) {
			Node *node = new Node(l->step());
			parent->add(node);
			parent = node;
		}
	}
	return *parent;
}


/**
 * Get a contextual property from a property list.
 * @param props		Property list to look in.
 * @param path		Contextual path.
 * @param id		Identifier of the looked property.
 * @return			Found property or null.
 */
const PropList& ContextualProperty::find(
	const PropList& props,
	const ContextualPath& path,
	const AbstractIdentifier& id
) {
	ContextualProperty *cprop = (ContextualProperty *)props.getProp(&ID);
	if(cprop)
		return cprop->findProps(props, path, id);
	else
		return props;
}


/**
 * Set a contextual property.
 * @param props		Property list to set in.
 * @param path		Contextual path of the property.
 * @param prop		Property to set.
 */
PropList& ContextualProperty::make(PropList& props, const ContextualPath& path) {
	if(!path.count())
		return props;
	ContextualProperty *cprop = (ContextualProperty *)props.getProp(&ID);
	if(!cprop) {
		cprop = new ContextualProperty();
		props.addProp(cprop);
	}
	return cprop->makeProps(path);
}


/**
 * Test if a contextual property exists, that is,
 * the property value is defined at any step of the given
 * contextual path.
 * @param props		Property list to look in.
 * @param path		Contextual path.
 * @param id		Looked identifier.
 */
bool ContextualProperty::exists(
	const PropList& props,
	const ContextualPath& path,
	const AbstractIdentifier& id
) {
	ContextualProperty *cprop = (ContextualProperty *)props.getProp(&ID);
	if(!cprop)
		return props.getProp(&id);
	else
		return cprop->findProps(props, path, id).getProp(&id);
}


/**
 * Obtain a reference on a property value in the given contextual path.
 * If the property does not exists, create and return it. If the property
 * exists at an intermediate level, copy it at the top level and
 * return reference to it.
 *
 * @param props		Property list to look in.
 * @param path		Contextual path.
 * @param id		Looked identifier.
 */
PropList& ContextualProperty::ref(
	PropList& props,
	const ContextualPath& path,
	const AbstractIdentifier& id
) {
	if(!path.count())
		return props;
	ContextualProperty *cprop = (ContextualProperty *)props.getProp(&ID);
	if(!cprop) {
		cprop = new ContextualProperty();
		props.addProp(cprop);
	}
	return cprop->refProps(props, path, id);
}


/**
 */
PropList& ContextualProperty::refProps(
	PropList& props,
	const ContextualPath& path,
	const AbstractIdentifier& id
) {

	// already existing ?
	PropList& res = make(props, path);
	if(res.getProp(&id))
		return res;

	// find top proplist and intermediate property
	Property *prop = props.getProp(&id);
	Node *parent = &root;
	for(int i = path.count() - 1; i >= 0; i--) {
		bool found = false;
		for(inhstruct::Tree::Iter child(parent); child; child++) {
			Node *node = (Node *)*child;
			if(node->step == path[i]) {
				parent = node;
				found = true;
				break;
			}
		}
		if(!found) {
			Node *node = new Node(path[i]);
			parent->add(node);
			parent = node;
		}
		Property *new_prop = parent->getProp(&id);
		if(new_prop)
			prop = new_prop;
	}

	// copy the property if found
	if(prop)
		parent->addProp(id.copy(prop));
	return *parent;
}


/**
 * Display the contextual information of the given property list.
 * @param out	Stream to output to.
 * @param props	Property list.
 */
void ContextualProperty::printFrom(io::Output& out, const PropList& props) {
	ContextualProperty *prop = (ContextualProperty *)props.getProp(&ID);
	if(!prop)
		out << "<no contextual information>\n";
	else
		prop->printRec(out, prop->root);
}


/**
 */
void ContextualProperty::printRec(io::Output& out, const Node& node, int indent) const {
	for(int i = 0; i < indent; i++)
		out << '\t';
	out << node.step << ' ' << (void *)&(const PropList&)node << ' ';
	node.print(out); out << '\n';
	for(inhstruct::Tree::Iter child(&node); child; child++) {
		Node *node = (Node *)*child;
		printRec(out, *node, indent + 1);
	}
}



/**
 * @class ContextualList
 * Represent a list node in a path describing a context path.
 *
 * @ingroup context
 */


/**
 * Count the number of node sin the list.
 * @return	Number of nodes.
 */
int ContextualList::count(void) const {
	int c = 0;
	for(const ContextualList *l = this; l; l = &l->next())
		c++;
	return c;
}


/**
 * Get the ith element in the current node list.
 * @note 		First element has an index i of 0.
 * @param i		Element number to look for.
 * @return		Step corresponding to the element number.
 */
const ContextualStep& ContextualList::ith(int i) const {
	for(const ContextualList *l = this; l; l = &l->next(), i--)
		if(!i)
			return l->step();
	ASSERTP(false, "out of bound index in ContextualList::ith");
	return ContextualStep::null;
}


/**
 * Null path.
 */
ContextualStep ContextualStep::null;


/**
 * Private identifier for contextual properties.
 */
AbstractIdentifier ContextualProperty::ID("ContextualProperty::ID");


/**
 * @class ContextualPath
 * A contextual path represents the chain of context containing a particular entity.
 * It is composed of ContextualStep summing up function, call and loop traversed to reach
 * the entity. Contextual paths may be used as identifiers to access a property from a
 * PropList depending on the context.
 *
 * @ingroup context
 */

}	// otawa
