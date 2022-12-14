/*
 *	categories module implementation
 *	Copyright (c) 2012, IRIT UPS.
 *
 *	This file is part of OTAWA
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

#include <otawa/cache/features.h>

namespace otawa { namespace cache {

/**
 * @class CategoryStats
 * This class is used to store statistics about the categories about cache
 * accesses. It it provided by cache category builders.
 * @see CATBuilder, CAT2Builder
 */

/**
 */
CategoryStats::CategoryStats(void) {
	reset();
}

/**
 * Reset the statistics.
 */
void CategoryStats::reset(void) {
	_total = 0;
	_linked = 0;
	for(int i = 0; i <= NOT_CLASSIFIED; i++)
		counts[i] = 0;
}

/**
 * @fn  void CategoryStats::add(category_t cat) const;
 * Increment the counter for the given category.
 * @param cat	Category to increment the counter.
 */

/**
 * @fn void CategoryStats::addLinked(void);
 * Add a new linked l-block to the statistics.
 */

/**
 * @fn int CategoryStats::get(category_t cat) const;
 * Get the counter of a category.
 * @param cat	Category to get counter for.
 * @return		Category count.
 */

/**
 * @fn int CategoryStats::total(void) const;
 * Get the total count of categories.
 * @return		Category total count.
 */

/**
 * @fn int CategoryStats::linked(void) const;
 * Get the count of linked statistics.
 * @return	Linked l-block statistics.
 */


/**
 * Put in the statistics to get statistics about cache categories.
 *
 * @par Hooks
 * @li processor configuration property list
 */
p::id<CategoryStats *> CATEGORY_STATS("otawa::CATEGORY_STATS", 0);


/**
 * This feature asserts that a category is assigned at each l-block involved
 * in the current task.
 *
 * @par Properties
 * @li @ref CATEGORY (@ref LBlock).
 * @li @ref CATEGORY_HEADER
 */
p::feature ICACHE_CATEGORY_FEATURE("otawa::ICACHE_CATEGORY_FEATURE", new Maker<NoProcessor>());


/**
 * This property stores the instruction cache access category of L-Blocks.
 * This value may be :
 * @li ALWAYS_HIT : accesses to this l-block produces always a cache hit,
 * @li ALWAYS_MISS : accesses to this l-block produces always a cache miss,
 * @li FIRST_HIT : accesses to this l-block produces an cache hit at the first
 * 					iteration of the containing loop and cache misses in the
 * 					next iterations,
 * @li FIRST_MISS : accesses to this l-block produces a cache miss at the first
 * 					iteration of the containing loop and cache hits in the next
 * 					iterations,
 *
 * @par Hooks
 * @li @ref LBlock
 */
p::id<category_t> CATEGORY("otawa::category", INVALID_CATEGORY);


/**
 *
 * In the case of a FIRST_HIT (or FIRST_MISS) property, contains the header
 * of the loop causing the HIT (or MISS) at the first iteration.
 *
 * @par Hooks
 * @li @ref LBlocks
 */
p::id<Block*> CATEGORY_HEADER("otawa::CATEGORY_HEADER", 0);


/**
 * @typedef category_t
 * Represents the categories associated to cache blocks to qualify
 * its behavior in cache.
 */

/**
 * @var INVALID_CATEGORY;
 * Special invalid value for a category.
 *
 * @var ALWAYS_HIT
 * Category set when a block access always results in a hit.
 *
 * @var FIRST_HIT
 * Category to qualify a block access that results first to a hit
 * and which following access have an unknown behavior (rarely used).
 *
 * @var FIRST_MISS
 * @deprecated Same as PERSISTENT.
 *
 * @var PERSISTENT
 * This category name is misleading: in fact, it qualifies a block which
 * first access result is unknown and following accesses are hits.
 *
 * @var ALWAYS_MISS
 * Category set to a cache block which access always results in a miss.
 * 
 * @var NOT_CLASSIFIED
 * Category set to a cache block which accesses cannot be assigned to
 * another category.
 */

/**
 */
io::Output& operator<<(io::Output& out, category_t cat) {
	static cstring names[] = {
		"invalid",
		"AH",
		"first-hit",
		"PE",
		"AM",
		"NC"
	};
	ASSERT(cat >= 0 || cat < sizeof(names) / sizeof(cstring));
	return out << names[cat];
}


/**
 */
io::Output& operator<<(io::Output& out, const CategoryStats& stats) {

	for(int i = ALWAYS_HIT; i <= NOT_CLASSIFIED; i++)
		out << category_t(i) << '\t' << (float(stats.get(category_t(i))) * 100 / stats.total())
			<< "% (" << stats.get(category_t(i)) << ")\n";
	out << "total\t\t100% (" << stats.total() << ")\n";
	out << "linked\t\t" << (float(stats.linked()) * 100 / stats.total())
		<< "% (" << stats.linked() << ")\n";
	return out;
}

}

Identifier<cache::category_t>& CATEGORY = cache::CATEGORY;
Identifier<Block *>& CATEGORY_HEADER = cache::CATEGORY_HEADER;
Identifier<cache::CategoryStats *>& CATEGORY_STATS = cache::CATEGORY_STATS;

}	// otawa::cache
