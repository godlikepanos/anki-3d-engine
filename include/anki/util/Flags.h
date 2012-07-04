#ifndef ANKI_UTIL_FLAGS_H
#define ANKI_UTIL_FLAGS_H

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
	void disableFlag(Value flag)
	{
		mask &= ~flag;
	}
	bool isFlagEnabled(Value flag) const
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
