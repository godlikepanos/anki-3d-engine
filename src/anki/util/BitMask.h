// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/StdTypes.h>

namespace anki
{

/// @addtogroup util_containers
/// @{

/// Easy bit mask manipulation.
template<typename T>
class BitMask
{
public:
	using Value = T;

	/// Default contructor. Will zero the mask.
	BitMask()
		: m_bitmask(static_cast<Value>(0))
	{
	}

	/// Construcor.
	/// @param mask The bits to enable.
	template<typename TInt>
	BitMask(TInt mask)
		: m_bitmask(static_cast<Value>(mask))
	{
	}

	/// Copy.
	BitMask(const BitMask& b) = default;

	/// Copy.
	BitMask& operator=(const BitMask& b) = default;

	/// Set or unset a bit at the given position.
	template<typename TInt>
	void set(TInt mask, Bool setBits = true)
	{
		Value maski = static_cast<Value>(mask);
		m_bitmask = (setBits) ? (m_bitmask | maski) : (m_bitmask & ~maski);
	}

	/// Unset a bit (set to zero) at the given position.
	template<typename TInt>
	void unset(TInt mask)
	{
		Value maski = static_cast<Value>(mask);
		m_bitmask &= ~maski;
	}

	/// Flip the bits at the given position. It will go from 1 to 0 or from 0 to
	/// 1.
	template<typename TInt>
	void flip(TInt mask)
	{
		Value maski = static_cast<Value>(mask);
		m_bitmask ^= maski;
	}

	/// Return true if the bit is set or false if it's not.
	template<typename TInt>
	Bool get(TInt mask) const
	{
		Value maski = static_cast<Value>(mask);
		return (m_bitmask & maski) == maski;
	}

	/// Given a mask check if any are enabled.
	template<typename TInt>
	Bool getAny(TInt mask) const
	{
		Value maski = static_cast<Value>(mask);
		return (m_bitmask & maski) != static_cast<Value>(0);
	}

protected:
	Value m_bitmask;
};
/// @}

} // end namespace anki
