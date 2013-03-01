#ifndef ANKI_BITSET_H
#define ANKI_BITSET_H

#include "anki/util/StdTypes.h"

namespace anki {

/// @addtogroup util
/// @{
/// @addtogroup containers
/// @{

/// Easy bit manipulation
template<typename T>
class Bitset
{
public:
	typedef T Value;

	Bitset()
		: bitmask(0)
	{}

	Bitset(T bitmask_)
		: bitmask(bitmask_)
	{}

	/// @name Bits manipulation
	/// @{
	void enableBits(Value mask)
	{
		bitmask |= mask;
	}
	void enableBits(Value mask, Bool enable)
	{
		bitmask = (enable) ? bitmask | mask : bitmask & ~mask;
	}

	void disableBits(Value mask)
	{
		bitmask &= ~mask;
	}

	void switchBits(Value mask)
	{
		bitmask ^= mask;
	}

	Bool bitsEnabled(Value mask) const
	{
		return bitmask & mask;
	}

	Value getBitmask() const
	{
		return bitmask;
	}
	/// @}

protected:
	Value bitmask;
};

/// @}
/// @}

} // end namespace anki

#endif
