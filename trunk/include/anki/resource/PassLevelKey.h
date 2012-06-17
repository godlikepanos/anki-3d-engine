#ifndef ANKI_RESOURCE_PASS_LEVEL_KEY_H
#define ANKI_RESOURCE_PASS_LEVEL_KEY_H

#include <unordered_map>
#include <cstdint>
#include <cstdlib>

namespace anki {

/// XXX
struct PassLevelKey
{
	uint32_t pass = 0;
	uint32_t level = 0;

	PassLevelKey()
	{}

	PassLevelKey(const PassLevelKey& b)
		: pass(b.pass), level(b.level)
	{}

	PassLevelKey(uint32_t pass_, uint32_t level_)
		: pass(pass_), level(level_)
	{}
};

/// Create hash functor
struct PassLevelKeyCreateHash
{
	size_t operator()(const PassLevelKey& b) const
	{
		return b.pass * 1000 + b.level;
	}
};

/// Values comparisons functor
struct PassLevelKeyComparision
{
	bool operator()(const PassLevelKey& a,
		const PassLevelKey& b) const
	{
		return a.pass == b.pass && a.level == b.level;
	}
};

/// Define an unorderer map with key the PassLevelKey and type given by a
/// template parameter
template<typename T>
using PassLevelHashMap = std::unordered_map<
	PassLevelKey, T, PassLevelKeyCreateHash, PassLevelKeyComparision>;

} // end namespace

#endif
