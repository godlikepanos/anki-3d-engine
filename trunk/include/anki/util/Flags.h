#ifndef ANKI_UTIL_FLAGS_H
#define ANKI_UTIL_FLAGS_H

namespace anki {

/// Easy flag manipulation
template<typename T>
class Flags
{
public:
	typedef T Value;

	/// @name Flag manipulation
	/// @{
	void enableFlag(Value flag)
	{
		flags |= flag;
	}
	void disableFlag(Value flag)
	{
		flags &= ~flag;
	}
	bool isFlagEnabled(Value flag) const
	{
		return flags & flag;
	}
	Value getFlagsBitmask() const
	{
		return flags;
	}
	/// @}

protected:
	Value flags = 0;
};

} // end namespace anki

#endif
