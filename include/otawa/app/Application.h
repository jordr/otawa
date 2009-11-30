/*
 *	$Id$
 *	Application class interface
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
#ifndef OTAWA_APP_APPLICATION_H_
#define OTAWA_APP_APPLICATION_H_

#include <elm/options.h>
#include <otawa/otawa.h>

namespace otawa {

using namespace elm;

// Application class
class Application: public option::Manager {
public:
	Application(
		cstring _program,
		Version _version = Version::ZERO,
		cstring _description = "",
		cstring _author = "",
		cstring _copyright = ""
	);
	virtual ~Application(void);
	
	int run(int argc, char **argv);	

protected:
	virtual void prepare(PropList& props);	
	virtual void work(PropList& props) throw(elm::Exception);
	virtual void work(const string& entry, PropList &props) throw(elm::Exception);

	inline WorkSpace *workspace(void) const { return ws; }
	void require(AbstractFeature&  feature);
	virtual void process(string arg);

private:
	option::BoolOption help, verbose;
	system::Path path;
	genstruct::Vector<string> entries;
	PropList props;
	PropList *props2;
	int result;
	WorkSpace *ws;
};

}	// otawa

// application macro
#define OTAWA_RUN(name) \
	int main(int argc, char **argv) { \
		name app; \
		return app.run(argc, argv); \
	}

#endif /* OTAWA_APP_APPLICATION_H_ */
