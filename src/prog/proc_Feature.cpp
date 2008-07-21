/*
 *	$Id$
 *	feature module interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-08, IRIT UPS.
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

#include <otawa/proc/Feature.h>

namespace otawa {


/**
 * @class AbstractFeature
 * See @ref Feature.
 */

/**
 * Build a simple feature.
 * @param name Name of the feature (only for information).
 */
AbstractFeature::AbstractFeature(CString name)
: Identifier<Processor *>(name, 0) {
}


/**
 * @fn void AbstractFeature::process(FrameWork *fw, const PropList& props) const;
 * This method is called, when a feature is not available, to provided a
 * default implementation of the feature.
 * @param fw	Framework to work on.
 * @param props	Property list configuration.
 */


/**
 * @fn void AbstractFeature::check(FrameWork *fw) const;
 * Check if the framework really implement the current feature. If not, it
 * throws a @ref ProcessorException. This method is only usually called
 * for debugging purpose as its execution is often very large.
 */


/**
 * @fn void AbstractFeature::clean(WorkSpace *ws) const;
 * This method is called each time a feature is invalidated. In this case, the
 * feature must removed all properties put on the workspace.
 */


/**
 * @class Feature
 * A feature is a set of facilities, usually provided using properties,
 * available on a framework. If a feature is present on a framework, it ensures
 * that the matching properties all also set.
 * @p
 * Most of the time, the features are provided and required by the code
 * processor. If a feature is not available, a processor matching the feature
 * is retrieved from the processor configuration property list if any or
 * from a default processor tied to the @ref Feature class. This processor
 * gives a default implementation computing the lacking feature. 
 * 
 * @param T	Default processor to compute the feature.
 * @param C Feature checker. This type (class or structure) must provide
 * a function called "check" as provided by the @ref Feature class.
 */


/**
 * @fn Feature::GenFeature(CString name);
 * Build a feature.
 * @param name	Feature name.
 */

} // otawa
