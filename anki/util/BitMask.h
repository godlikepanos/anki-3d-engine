// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
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
		: m_bitmask(0)
	{
	}

	/// Construcor.
	/// @param mask The bits to enable.
	BitMask(Value mask)
		: m_bitmask(mask)
	{
	}

	/// Copy.
	BitMask(const BitMask& b) = default;

	/// Copy.
	BitMask& operator=(const BitMask& b) = default;

	/// Set or unset a bit at the given position.
	void set(Value mask, Bool setBits = true)
	{
		m_bitmask = (setBits) ? (m_bitmask | mask) : (m_bitmask & ~mask);
	}

	/// Unset a bit (set to zero) at the given position.
	void unset(Value mask)
	{
		m_bitmask &= ~mask;
	}

	/// Flip the bits at the given position. It will go from 1 to 0 or from 0 to 1.
	void flip(Value mask)
	{
		m_bitmask ^= mask;
	}

	/// Return true if the bit is set or false if it's not.
	Bool get(Value mask) const
	{
		return (m_bitmask & mask) == mask;
	}

	/// Given a mask check if any are enabled.
	Bool getAny(Value mask) const
	{
		return (m_bitmask & mask) != 0;
	}

protected:
	Value m_bitmask;
};
/// @}

} // end namespace anki
