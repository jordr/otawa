/*
 *	XIterativeDFA class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 200--17, IRIT UPS.
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
#include <otawa/dfa/XIterativeDFA.h>

namespace otawa { namespace dfa {

/**
 * @class XIterativeDFA
 * The DFAEngine implements an extended Iterative Data Flow Algorithm, that is,
 * it work on graphs with any forms. Basically, it just apply the GEN/KILL
 * computation to a set of nodes until reaching a fix point. 
 * @par
 * OUT = union(IN) inter KILL union GEN
 * @par
 * But, to traverse the graph, it uses a visitor giving the next nodes to
 * process. With a visitor like @ref dfa::XCFGVisitor, it may applied on a whole
 * CFG hierarchy to perform inter-procedure analysis.
 * 
 * @param V: the used visitor type. It must have the following signature
 * (nodes are identified by an integer index).
 *
 * The V parameter must implement the following concept:
 * @code
 * class V {
 * public:
 * 	typedef domain_t;			// domain type
 *  domain_t *empty(void);		// return an empty domain
 * 	domain_t *gen(int node);	// return the GEN value
 *	domain_t *kill(int node);	// return the KILL value
 *	void free(domain_t *d);		// free the given domain
 *
 * 	typedef key_t;					// node key
 *  int index(const key_t& key);	// get index of the given key  
 * 
 * 	int size(void);	// count of graph nodes
 * 	void visitPreds(XIterativeDFA<> engine, int node);
 * 		// visit the predecessors of a node, call nextPred(int node) on engine.
 *	void visitSuccs(XIterativeDFA<> engine, int node);
 * 		// visit the successors of a node, call nextSucc(int node) on engine.
 * };
 * @endcode
 * 
 * The V::domain_t type must implements the following concept :
 * @code
 * class domain_t {
 *	void reset(void);			// reset the domain 
 *  void join(domain_t *d);		// join between current domain and given one.
 *  void meet(domain_t *d);		// meet between current domain and given one.
 *	bool equals(domain_t *d);	// test if both domains are equals
 * };
 * @endcode 
 * 
 * Known visitors includes:
 * @li @ref dfa::XCFGVisitor
 */


/**
 * @fn XIterativeDFA::XIterativeDFA(V& visitor);
 * Build an iterative DFA with the given visitor.
 * @param visitor	Visitor to use.
 */


/**
 * @fn void XIterativeDFA::proceed(void);
 * Perform the DFA computation until reaching a fixpoint.
 */


/**
 * @fn void XIterativeDFA::nextPred(int pred);
 * This functions is used internally to communicate with the visitor. When
 * the visitor function visitPreds() is called, the visitor will call this
 * functione each time a predecessor is found.
 * @param pred	Current predecessor. 
 */


/**
 * @fn V::t *XIterativeDFA::in(V::key_t key);
 * Get the IN set of the given node.
 * @param key	Identifier of the current node.
 * @return		IN set.
 */


/**
 * @fn V::t *XIterativeDFA::out(V::key_t key);
 * Get the OUT set of the given node.
 * @param key	Identifier of the current node.
 * @return		OUT set.
 */

/**
 * @fn V::t *XIterativeDFA::gen(V::key_t key);
 * Get the GEN set of the given node.
 * @param key	Identifier of the current node.
 * @return		GEN set.
 */

/**
 * @fn V::t *XIterativeDFA::kill(V::key_t key);
 * Get the KILL set of the given node.
 * @param key	Identifier of the current node.
 * @return		KILL set.
 */

} } // otawa::dfa
