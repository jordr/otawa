/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	program.cc -- program classes implementation.
 */

#include <otawa/program.h>

namespace otawa {

/**
 * @class ProgObject
 * This is the base class of all objects representing program parts.
 * It provides facilities for storing and retrieving bound special properties.
 */

/**
 * @class ProgItem
 * Base class of the components of a program file segment.
 */

/**
 * @fn ProgItem::~ProgItem(void);
 * Protected destructor for avoiding implementation unexpected deletion.
 */
 
/**
 * @fn CString ProgItem::name(void);
 * Get the name of the program item if some is defined. It may be the name
 * of a function for a piece of code or the name of a data.
 * @return Name ofthis item or an empty string else.
 */

/**
 * @fn address_t ProgItem::address(void);
 * Get the address of the item if some has been assigned.
 * @return Address of the item or address 0 if none is assigned.
 * @note In workstation systems, it is commonly accepted that the address
 * 0 is ever invalid because it is the usual value of NULL in C. It should also
 * work the same for embedded systems.
 */
 
 /**
  * @fn size_t ProgItem::size(void);
  * Get the size of the item in bytes.
  * @return	Size of the item.
  */

/**
 *	@fn Code *ProgItem::toCode(void);
 *	Get the code program item if it is, null else.
 *	@return Code program item or null.
 */

/**
 *	@fn Data *ProgItem::toData(void);
 *	Get the data program item if it is, null else.
 *	@return Data program item or null.
 */


/**
 * @class Code
 * This program item represents a piece of code. It is usually a function but
 * it may be something else according the processed language or the optimization
 * methods.
 */

/**
 * @fn Inst *Code::first(void) const;
 * Get the first instruction in the code.
 * @return First instruction.
 */

/**
 * @fn Inst *Code::last(void) const;
 * Get the last instruction in the code.
 * @return Last instruction.
 */


/**
 * @class Data
 * This class represent a data item in the program. According information
 * available to Otawa, it may represent a block of anonymous data or 
 * may match some real variable from the underlying programming language.
 */

/**
 * @fn Type *Data::getType(void);
 * Get the type of the data block.
 * @return Type of the data block. Even when the type is not explicitly known,
 * a byte block type object is returned.
 */


/**
 * @class Segment
 * @par In usual file format like ELF, COFF and so on, the program file is
 * divided in segment according platform needs or memory propertes.
 * @par Usually, we find a ".text" segment containing program code,
 * ".data" containing initialized data, ".bss" containing uninitialized data,
 * ".rodata" containing read-only data. Yet, more segments may be available.
 * @par Making the segment an abstract class allows the platform applying
 * constraints to its address and size.
 */

/**
 * @fn Segment::~Segment(void);
 * Protected destructor for avoiding implementation unexpected deletion.
 */

/**
 * @fn CString Segment::name(void);
 * Get tne name of the segment.
 * @return Name of the segment.
 */

/**
 * @fn address_t Segment::address(void);
 * Get the base address of the segment.
 * @return Base address of the segment or 0 if no address has been assigned
 * to the segment.
 */
 
/**
 * @fn size_t Segment::size(void);
 * Get the size of the segment.
 * @return Size of the segment.
 */

/**
 * @fn elm::Collection<ProgItem *> Segment::items(void);
 * Get the items contained in the segment.
 * @return Collection of items in the segment.
 */

/**
 * @fn int Segment::flags(void);
 * Get flag information about the segment. This flags are composed by OR'ing the constants
 * EXECUTABLE and WRITABLE.
 * @return Flags value.
 */

/**
 * @class File
 * This class represents a file involved in the building of a process. A file
 * usually matches a program file on the system file system.
 */

/**
 * @fn CString File::name(void);
 * Get the name of the file. It is usually its absolute path.
 * @return Name of the file.
 */

/**
 * @fn const elm::Collection<Segment *> File::segments(void) const;
 * Get the segments composing the files.
 * @return Collection of the segments in the file.
 */

/**
 * Property with this identifier is put on instructions or basic blocks which a symbol is known for.
 * Its property is of type String.
 */
id_t File::ID_Label = Property::getID("otawa.File.Label");

/**
 * @fn address_t File::findLabel(const String& label);
 * Find the address of the given label.
 * @param label Label to find.
 * @return	Address of the label or null if label is not found.
 */


/**
 * @fn Symbol *File::findSymbol(String name);
 * Find a symbol  by its name.
 * @param name	Symbol name.
 * @return		Found symbol or null.
 */


/**
 * @fn elm::Collection<Symbol *>& symbols(void) ;
 * Get the collection of existing symbols.
 * @return	Symbol collection.
 */

}; // namespace otawa
