/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/hardware/CacheConfiguration.h -- CacheConfiguration class interface.
 */
#ifndef OTAWA_HARDWARE_CONFIGURATION_CACHE_H
#define OTAWA_HARDWARE_CONFIGURATION_CACHE_H

#include <otawa/hardware/Cache.h>

namespace otawa {

// CacheConfiguration class
class CacheConfiguration {
	const Cache *icache, *dcache;
public:
	static const CacheConfiguration NO_CACHE;
	inline CacheConfiguration(const Cache *inst_cache = 0,
		const Cache *data_cache = 0);
	inline const Cache *instCache(void) const;
	inline const Cache *dataCache(void) const;
	inline bool hasInstCache(void) const;
	inline bool hasDataCache(void) const;
	inline bool isUnified(void) const;
	inline bool isHarvard(void) const;
};

// Inlines
inline CacheConfiguration::CacheConfiguration(const Cache *inst_cache,
const Cache *data_cache): icache(inst_cache), dcache(data_cache) {
}

inline const Cache *CacheConfiguration::instCache(void) const {
	return icache;
}

inline const Cache *CacheConfiguration::dataCache(void) const {
	return dcache;
}

inline bool CacheConfiguration::hasInstCache(void) const {
	return icache != 0;
}

inline bool CacheConfiguration::hasDataCache(void) const {
	return dcache != 0;
}

inline bool CacheConfiguration::isUnified(void) const {
	return icache == dcache;
}

inline bool CacheConfiguration::isHarvard(void) const {
	return icache != dcache;
}

} // otawa

#endif	// OTAWA_HARDWARE_CONFIGURATION_CACHE_H
