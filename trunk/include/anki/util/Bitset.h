// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_BITSET_H
#define ANKI_BITSET_H

#include "anki/util/StdTypes.h"

namespace anki {

/// @addtogroup util_containers
/// @{

/// Easy bit manipulation
template<typename T>
class Bitset
{
public:
	typedef T Value;

	Bitset()
		: m_bitmask(0)
	{}

	Bitset(T bitmask)
		: m_bitmask(bitmask)
	{}

	/// @name Bits manipulation
	/// @{
	void enableBits(Value mask)
	{
		m_bitmask |= mask;
	}
	void enableBits(Value mask, Bool enable)
	{
		m_bitmask = (enable) ? m_bitmask | mask : m_bitmask & ~mask;
	}

	void disableBits(Value mask)
	{
		m_bitmask &= ~mask;
	}

	void switchBits(Value mask)
	{
		m_bitmask ^= mask;
	}

	Bool bitsEnabled(Value mask) const
	{
		return m_bitmask & mask;
	}

	Value getBitmask() const
	{
		return m_bitmask;
	}
	/// @}

protected:
	Value m_bitmask;
};

/// @}

} // end namespace anki

#endif
