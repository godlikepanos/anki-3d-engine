#ifndef ANKI_UTIL_FLAGS_H
#define ANKI_UTIL_FLAGS_H

#include "anki/util/StdTypes.h"

namespace anki {

/// Easy flag manipulation
template<typename T>
class Flags
{
public:
	typedef T Value;

	Flags()
	{}

	Flags(T bitmask_)
		: bitmask(bitmask_)
	{}

	/// @name Flag manipulation
	/// @{
	void enableFlags(Value mask)
	{
		bitmask |= mask;
	}
	void enableFlags(Value mask, Bool enable)
	{
		bitmask = (enable) ? bitmask | mask : bitmask & ~mask;
	}

	void disableFlags(Value mask)
	{
		bitmask &= ~mask;
	}

	void switchFlags(Value mask)
	{
		bitmask ^= mask;
	}

	Bool flagsEnabled(Value mask) const
	{
		return bitmask & mask;
	}

	Value getFlagsBitbitmask() const
	{
		return bitmask;
	}
	/// @}

protected:
	Value bitmask = 0;
};

} // end namespace anki

#endif
