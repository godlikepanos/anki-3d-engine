#ifndef ANKI_UTIL_ARRAY_H
#define ANKI_UTIL_ARRAY_H

#include "anki/util/Assert.h"
#include <array>

namespace anki {

/// @addtogroup util
/// @{
template<typename T, size_t size>
class Array: public std::array<T, size>
{
public:
	typedef std::array<T, size> Base;

#if !NDEBUG
	typename Base::reference operator[](typename Base::size_type n)
	{
		ANKI_ASSERT(n < Base::size());
		return Base::operator[](n);
	}

	typename Base::const_reference operator[](
		typename Base::size_type n) const
	{
		ANKI_ASSERT(n < Base::size());
		return Base::operator[](n);
	}
#endif
};
/// @}

} // end namespace anki

#endif
