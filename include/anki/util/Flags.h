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

	Flags(T mask_)
		: mask(mask_)
	{}

	/// @name Flag manipulation
	/// @{
	void enableFlag(Value flag)
	{
		mask |= flag;
	}
	void enableFlag(Value flag, Bool enable)
	{
		mask = (enable) ? mask | flag : mask & ~flag;
	}
	void disableFlag(Value flag)
	{
		mask &= ~flag;
	}
	Bool isFlagEnabled(Value flag) const
	{
		return mask & flag;
	}
	Value getFlagsBitmask() const
	{
		return mask;
	}
	/// @}

protected:
	Value mask = 0;
};

} // end namespace anki

#endif
