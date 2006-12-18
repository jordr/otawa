/*
 * $Id$
 * Copyright (c) 2006 IRIT-UPS
 *
 * XIterativeDFA class interface.
 */
#ifndef OTAWA_DFA_XITERATIVEDFA_H
#define OTAWA_DFA_XITERATIVEDFA_H

#include <assert.h>

namespace otawa { namespace dfa {

// XIterativeDFA class
template <class V>
class XIterativeDFA {
	V& visit;
	int size;
	typename V::domain_t **outs, **gens, **kills, *new_out, **ins;
public:
	inline XIterativeDFA(V& visitor);
	inline ~XIterativeDFA(void);
	inline void process(void);
	inline void nextPred(int pred);
	inline typename V::domain_t *in(const typename V::key_t& key)
		{ return ins[visit.index(key)]; }
	inline typename V::domain_t *out(const typename V::key_t& key)
		{ return outs[visit.index(key)]; }
	inline typename V::domain_t *gen(const typename V::key_t& key)
		{ return gens[visit.index(key)]; }
	inline typename V::domain_t *kill(const typename V::key_t& key)
		{ return kills[visit.index(key)]; }
};


// Inlines
template <class V>
inline XIterativeDFA<V>::XIterativeDFA(V& visitor)
: visit(visitor), ins(0) {
	size = visit.size();
	ins = new typename V::domain_t *[size];
	outs = new typename V::domain_t *[size];
	gens = new typename V::domain_t *[size];
	kills = new typename V::domain_t *[size];
	for(int i = 0; i < size; i++) {
		ins[i] = visit.empty();
		outs[i] = visit.empty();
		gens[i] = visit.gen(i);
		kills[i] = visit.kill(i);
	}
}

template <class V>
inline XIterativeDFA<V>::~XIterativeDFA(void) {
	for(int i = 0; i < size; i++) {
		visit.free(ins[i]);
		visit.free(outs[i]);
		visit.free(gens[i]);
		visit.free(kills[i]);
	}
	delete [] ins;
	delete [] outs;
	delete [] gens;
	delete [] kills;
}

template <class V>
inline void XIterativeDFA<V>::nextPred(int pred) {
	assert(pred < size);
	new_out->join(outs[pred]);
}

template <class V>
inline void XIterativeDFA<V>::process(void) {
	bool fixpoint = false;
	typename V::domain_t *tmp;
	new_out = visit.empty();
	while(!fixpoint) {
		fixpoint = true;
		for(int i = 0; i < size; i++) {
			visit.visitPreds(*this, i);
			tmp = new_out;
			new_out = ins[i];
			ins[i] = tmp;
			new_out->reset();
			new_out->join(ins[i]);
			new_out->meet(kills[i]);
			new_out->join(gens[i]);
			if(!new_out->equals(outs[i])) {
				fixpoint = false;
				tmp = new_out;
				new_out = outs[i];
				outs[i] = tmp;
				new_out->reset();
			}
		}
	}
	visit.free(new_out);
}

} } // otawa::dfa

#endif // OTAWA_DFA_XITERATIVEDFA_H
