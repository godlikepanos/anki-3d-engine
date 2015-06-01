// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_BITSET_H
#define ANKI_BITSET_H

#include "anki/util/StdTypes.h"

namespace anki {

/// @addtogroup util_containers
/// @{

/// Easy bit manipulation.
template<typename T>
class Bitset
{
public:
	using Value = T;

	Bitset()
		: m_bitmask(static_cast<Value>(0))
	{}

	Bitset(Value bitmask)
		: m_bitmask(bitmask)
	{}

	/// @name Bits manipulation
	/// @{
	template<typename Y>
	void enableBits(Y mask)
	{
		Value maski = static_cast<Value>(mask);
		m_bitmask |= maski;
	}

	template<typename Y>
	void enableBits(Y mask, Bool enable)
	{
		Value maski = static_cast<Value>(mask);
		m_bitmask = (enable) ? (m_bitmask | maski) : (m_bitmask & ~maski);
	}

	template<typename Y>
	void disableBits(Y mask)
	{
		Value maski = static_cast<Value>(mask);
		m_bitmask &= ~maski;
	}

	template<typename Y>
	void switchBits(Y mask)
	{
		Value maski = static_cast<Value>(mask);
		m_bitmask ^= maski;
	}

	template<typename Y>
	Bool bitsEnabled(Y mask) const
	{
		Value maski = static_cast<Value>(mask);
		return (m_bitmask & maski) == maski;
	}

	template<typename Y>
	Bool anyBitsEnabled(Y mask) const
	{
		Value maski = static_cast<Value>(mask);
		return (m_bitmask & maski) != static_cast<Value>(0);
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
