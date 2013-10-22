#ifndef ANKI_RESOURCE_PASS_LEVEL_KEY_H
#define ANKI_RESOURCE_PASS_LEVEL_KEY_H

#include "anki/util/StdTypes.h"
#include "anki/util/Assert.h"
#include "anki/util/Array.h"

namespace anki {

/// The AnKi passes
enum Pass
{
	COLOR_PASS, ///< For MS
	DEPTH_PASS, ///< For shadows

	PASS_COUNT
};

/// Max level of detail
const U MAX_LOD = 3;

/// A key that consistst of the rendering pass and the level of detail
struct PassLodKey
{
	U8 pass;
	U8 level;

	PassLodKey()
		: pass(COLOR_PASS), level(0)
	{
		ANKI_ASSERT(level <= MAX_LOD);
	}

	PassLodKey(const PassLodKey& b)
		: pass(b.pass), level(b.level)
	{
		ANKI_ASSERT(level <= MAX_LOD);
	}

	explicit PassLodKey(const U8 pass_, const U8 level_)
		: pass(pass_), level(level_)
	{
		ANKI_ASSERT(level <= MAX_LOD);
	}
};

/// An array to store based on pass and LOD
template<typename T>
using PassLodArray = Array2d<T, PASS_COUNT, MAX_LOD + 1>;

} // end namespace anki

#endif
