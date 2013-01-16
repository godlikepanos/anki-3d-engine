#ifndef ANKI_UTIL_NON_COPYABLE_H
#define ANKI_UTIL_NON_COPYABLE_H

namespace anki {

/// @addtogroup util
/// @{
/// @addtogroup patterns
/// @{

/// Makes a derived class non copyable
struct NonCopyable
{
	NonCopyable()
	{}

	NonCopyable(const NonCopyable&) = delete;
	NonCopyable& operator=(const NonCopyable&) = delete;

	~NonCopyable()
	{}
};
/// @}
/// @}

} // end namespace anki

#endif
