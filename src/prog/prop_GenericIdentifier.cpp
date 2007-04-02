/*
 *	$Id$
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	otawa/prog/GenericIdentifier.cpp -- implementation of GenericIdentifier class.
 */

#include <otawa/prop/Identifier.h>
#include <otawa/prop/PropList.h>

namespace otawa {

/**
 * @class Identifier
 * This class represents identifier with a typed associated value.
 * Identifier with a value should declared with this kind of identifier because
 * it procides full support for reflexive facilities.
 * @param T	The type of data stored in a property with this identifier.
 * @p Note that the property management functions of this class are safer to
 * use than the rough @ref PropList functions because they ensure safe value
 * type management.
 */


/**
 * @fn Identifier<T>::Identifier(elm::CString name);
 * Build a new generic identifier.
 * @param name	Name of the generic identifier.
 */


/**
 * @fn Identifier<T>::Identifier(elm::CString name, const T& default_value);
 * Build a new generic identifier.
 * @param name			Name of the generic identifier.
 * @param default_value	Default value of the identifier.
 */


/**
 * @fn void Identifier<T>::add(PropList& list, const T& value);
 * Add a generic property to the given list with the current identifier.
 * @param list	List to add to.
 * @param value	Value of the property.
 */



/**
 * @fn void Identifier<T>::add(PropList& list, const T& value) const;
 * Add a generic property to the given list with the current identifier.
 * @param list	List to add to.
 * @param value	Value of the property.
 */


/**
 * @fn void Identifier<T>::add(PropList *list, const T& value) const;
 * Add a generic property to the given list with the current identifier.
 * @param list	List to add to.
 * @param value	Value of the property.
 */


/**
 * @fn void Identifier<T>::set(PropList& list, const T& value) const;
 * Set the value of a generic property with the current identifier to the given list.
 * @param list	List to set in.
 * @param value	Value to set.
 */


/**
 * @fn void Identifier<T>::set(PropList *list, const T& value) const;
 * Set the value of a generic property with the current identifier to the given list.
 * @param list	List to set in.
 * @param value	Value to set.
 */


/**
 * @fn const T& Identifier<T>::get(const PropList& list, const T& def) const;
 * Get the value associated with a property matching the current identifier.
 * If the property is not found, return the default value.
 * @param list	List to look in.
 * @param def	Default value.
 * @return		Found value or the default value.
 */


/**
 * @fn const T& Identifier<T>::get(const PropList *list, const T& def) const;
 * Get the value associated with a property matching the current identifier.
 * If the property is not found, return the default value.
 * @param list	List to look in.
 * @param def	Default value.
 * @return		Found value or the default value.
 */


/**
 * @fn const T& Identifier<T>::use(const PropList& list) const;
 * Get the value matching the current identifier in the given list.
 * Cause a run-time abort if the property is not available.
 * @param list	List to look in.
 * @return		Matching value.
 */


/**
 * @fn const T& Identifier<T>::use(const PropList *list) const;
 * Get the value matching the current identifier in the given list.
 * Cause a run-time abort if the property is not available.
 * @param list	List to look in.
 * @return		Matching value.
 */



/**
 * @fn const T& Identifier<T>::value(const PropList& list) const;
 * For internal use only.
 * @internal Same as get() without default value. Only provided for symmetry.
 */


/**
 * @fn const T& Identifier<T>::value(const PropList *list) const;
 * For internal use only.
 * @internal Same as get() without default value. Only provided for symmetry.
 */


/**
 * @fn Value Identifier<T>::value(PropList& list) const;
 * For internal use only.
 * @internal Provide an assignable value.
 */


/**
 * @fn Value Identifier<T>::value(PropList *list) const;
 * For internal use only.
 * @internal Provide an assignable value.
 */


/**
 * @fn const T& Identifier<T>::operator()(const PropList& props) const;
 * Read the value in a functional way.
 * @param props	Property list to read the property from.
 * @return		Value of the property matching the current identifier in
 * the list.
 */


/**
 * @fn const T& Identifier<T>::operator()(const PropList *props) const;
 * Read the value in a functional way.
 * @param props	Property list to read the property from.
 * @return		Value of the property matching the current identifier in
 * the list.
 */


/**
 * @fn Value Identifier<T>::operator()(PropList& props) const;
 * Read or write a property value in a functional way. The returned value
 * may be read (automatic conversion to the value) or written (using operator
 * = to set the value or += to add a new value at the property list.
 * @param props	Property list to read the property from.
 * @return		Value of the property matching the current identifier in
 * the list.
 */


/**
 * @fn Value Identifier<T>::operator()(PropList *props) const;
 * Read or write a property value in a functional way. The returned value
 * may be read (automatic conversion to the value) or written (using operator
 * = to set the value or += to add a new value at the property list.
 * @param props	Property list to read the property from.
 * @return		Value of the property matching the current identifier in
 * the list.
 */


// Identifier<T>::print Specializations
static void escape(elm::io::Output& out, char chr, char quote) {
	if(chr >= ' ') {
		if(chr == quote)
			out << '\\' << quote;
		else
			out << chr;
	}
	else
		switch(chr) {
		case '\n': out << "\\n"; break;
		case '\t': out << "\\t"; break;
		case '\r': out << "\\r"; break;
		default: out << "\\x" << io::hex((unsigned char)chr); break;
	}
}


template <>
void Identifier<char>::print(elm::io::Output& out, const Property& prop) const {
	out << '\'';
	escape(out, get(prop), '\'');
	out << '\'';
}


template <>
void Identifier<CString>::print(elm::io::Output& out, const Property& prop) const {
	out << '"';
	CString str = get(prop);
	for(int i = 0; str[i]; i++)
		escape(out, str[i], '"'); 
	out << '"';
}


template <>
void Identifier<elm::String>::print(elm::io::Output& out, const Property& prop) const {
	out << '"';
	const String& str = get(prop);
	for(int i = 0; i < str.length(); i++)
		escape(out, str[i], '"'); 
	out << '"';
}


template <>
void Identifier<PropList *>::print(elm::io::Output& out, const Property& prop) const {
	out << "proplist(" << &prop << ")";
}


// GenericIdentifier<T>::scan Specializations
template <>
void Identifier<CString>::scan(PropList& props, VarArg& args) const {
	props.set(*this, args.next<char *>());
}


template <>
void Identifier<String>::scan(PropList& props, VarArg& args) const {
	props.set(*this, args.next<char *>());
}


/**
 * @fn void Identifier::remove(PropList& list) const;
 * Remove the property with the current identifier from the given list.
 * @param list	List to remove from.
 */


/**
 * @fn bool Identifier::exists(PropList& list) const; 
 * Test if the given list contains a property with the current identifier.
 * @param list	List to look in.
 * @return		True if there is a property with current identifier, false else.
 */


/**
 * @fn void Identifier::remove(PropList *list) const;
 * Remove the property with the current identifier from the given list.
 * @param list	List to remove from.
 */


/**
 * @fn bool Identifier::exists(PropList *list) const; 
 * Test if the given list contains a property with the current identifier.
 * @param list	List to look in.
 * @return		True if there is a property with current identifier, false else.
 */

}	// otawa
