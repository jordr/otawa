/*
 *	TimeUnitTimer class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2019, IRIT UPS.
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
#ifndef OTAWA_ETIME_TIMEUNITTIMER_H_
#define OTAWA_ETIME_TIMEUNITTIMER_H_

#include "AbstractTimeBuilder.h"

namespace otawa { namespace etime {

class TimeUnitGenerator;

class TimeUnitTimer: public AbstractTimeBuilder {
public:
	static p::declare reg;
	TimeUnitTimer();
	~TimeUnitTimer();

private:
	TimeUnitGenerator *gen;
};

} }	// otawa::etime

#endif /* OTAWA_ETIME_TIMEUNITTIMER_H_ */
