/*
 *	Platform class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-13, IRIT UPS.
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
#ifndef OTAWA_HARD_PLATFORM_H
#define OTAWA_HARD_PLATFORM_H

#include <elm/system/Path.h>
#include <otawa/properties.h>
#include <otawa/hard/Register.h>
#include <otawa/prog/Manager.h>

// External classes
namespace elm {
	namespace xom {
		class Element;
	}
}

namespace otawa {

class Manager;

namespace hard {

// Extern Classes
class CacheConfiguration;
class Memory;
class Processor;

// Platform class
class Platform: public AbstractIdentifier {
public:
	typedef elm::genstruct::Table<const hard::RegBank *> banks_t;

	static const elm::String ANY;

	// Architecture
	static const elm::String POWERPC;

	// ABI
	static const elm::String ELF;
	static const elm::String EABI;
	static const elm::String LINUX;
	static const elm::String LINUX_2_4;
	static const elm::String LINUX_2_6;

	// Platform
	static const elm::String MAC;
	static const elm::String SIM;

	// Platform identification
	class Identification {
		elm::String _name;
		elm::String _arch;
		elm::String _abi;
		elm::String _mach;
		void split(void);
	public:
		Identification(const elm::String& name);
		Identification(const elm::String& arch, const elm::String& abi,
			const elm::String& mach = ANY);
		inline const elm::String& name(void) const { return _name; }
		inline const elm::String& architecture(void) const { return _arch; }
		inline const elm::String& abi(void) const { return _abi; }
		inline const elm::String& machine(void) const { return _mach; }
		bool matches(const Identification& id);
		Identification& operator=(const Identification& id);
		void print(io::Output& out) const;
	};

	// Constructors
	static const Identification ANY_PLATFORM;
	Platform(const Identification& id, const PropList& props = PropList::EMPTY);
	Platform(cstring name, const Identification& id, const PropList& props = PropList::EMPTY);
	Platform(const Platform& platform, const PropList& props = PropList::EMPTY);

	// Identification
	inline const Identification& identification(void) const { return id; }
	virtual bool accept(const Identification& id);
	inline bool accept(elm::CString name) { return accept(Identification(name)); }
	inline bool accept(const elm::String& name) { return accept(Identification(name)); }

	// Register bank access
	inline const banks_t& banks(void) const { return *_banks; }
	inline int regCount(void) const { return rcnt; }
	Register *findReg(int uniq) const;
	const Register *findReg(const string& name) const;
	virtual const Register *getSP(void) const;
	virtual const Register *getPC(void) const;

protected:
	friend class otawa::Manager;
	static const banks_t null_banks;
	void setBanks(const banks_t& banks);

private:
	Identification id;
	int rcnt;
	const banks_t *_banks;
};
inline io::Output& operator<<(io::Output& out, const Platform::Identification& id) { id.print(out); return out; }

} } // otawa::hard

#endif // OTAWA_HARD_PLATFORM_H
