#ifndef ANKI_RESOURCE_PASS_LEVEL_KEY_H
#define ANKI_RESOURCE_PASS_LEVEL_KEY_H

#include "anki/util/StdTypes.h"
#include <unordered_map>
#include <cstdlib>

namespace anki {

/// The AnKi passes
enum Pass
{
	COLOR_PASS, ///< For MS
	DEPTH_PASS, ///< For shadows

	/// For MS color pass after early Z. It's different than COLOR because it
	/// doesn't do any discard, earlyZ did it before
	//COLOR_EARLYZ_PASS,

	PASS_COUNT
};

/// A key that consistst of the rendering pass and the level of detail
struct PassLevelKey
{
	U8 pass;
	U8 level;

	PassLevelKey()
		: pass(COLOR_PASS), level(0)
	{}

	PassLevelKey(const PassLevelKey& b)
		: pass(b.pass), level(b.level)
	{}

	explicit PassLevelKey(const U8 pass_, const U8 level_)
		: pass(pass_), level(level_)
	{}
};

/// Create hash functor
struct PassLevelKeyCreateHash
{
	size_t operator()(const PassLevelKey& b) const
	{
		return ((U32)b.pass << 16) | (U32)b.level;
	}
};

/// Values comparisons functor
struct PassLevelKeyComparision
{
	Bool operator()(const PassLevelKey& a,
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

} // end namespace anki

#endif
