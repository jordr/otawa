/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	properties.h -- object properties interface.
 */
#ifndef OTAWA_PROPERTIES_H
#define OTAWA_PROPERTIES_H

#include <assert.h>
#include <elm/string.h>
#include <elm/utility.h>
using namespace elm;

namespace otawa {

// Base types
typedef unsigned long id_t;
const id_t INVALID_ID = 0;

// Property description
class Property {
	friend class PropList;
	Property *next;
	id_t id;
public:
	virtual ~Property(void) { };
	static id_t getID(CString name);
	inline Property(id_t _id): id(_id) { };
	inline Property(CString name): id(getID(name)) { };
	inline id_t getID(void) const { return id; };
	inline Property *getNext(void) const { return next; };
};

// GenericProperty class
template <class T> class GenericProperty: public Property {
	T value;
public:
	inline GenericProperty(id_t id, T _value): Property(id), value(_value) { };
	inline GenericProperty(CString name, T _value): Property(name), value(_value) { };
	inline T& getValue(void) { return value; };
};

// PropList class
class PropList {
	Property *head;
public:
	inline PropList(void): head(0) { };
	inline ~PropList(void) { clear(); };

	// Property access
	Property *getProp(id_t id);
	void setProp(Property *prop);
	void removeProp(id_t id);
	void clear(void);
	inline void setProp(id_t id) { setProp(new Property(id)); };
	
	// Property value access
	template <class T> inline T get(id_t id, const T def_value);
	template <class T> inline Option<T> get(id_t id);
	template <class T> inline T& use(id_t id);
	template <class T> inline void set(id_t id, const T value);
};


// PropList methods
template <class T> T PropList::get(id_t id, const T def_value) {
	Property *prop = getProp(id);
	return !prop ? def_value : ((GenericProperty<T> *)prop)->getValue();
};
template <class T> Option<T> PropList::get(id_t id) {
	Property *prop = getProp(id);
	return !prop ? Option<T>() : Option<T>(((GenericProperty<T> *)prop)->getValue());
};
template <class T> T& PropList::use(id_t id) {
	Property *prop = getProp(id);
	if(!prop)
		assert(0);
	return ((GenericProperty<T> *)prop)->getValue();
};
template <class T> void PropList::set(id_t id, const T value) {
	setProp(new GenericProperty<T>(id, value));
};

};

#endif		// PROJECT_PROPERTIES_H
