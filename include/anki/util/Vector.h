#ifndef ANKI_UTIL_VECTOR_H
#define ANKI_UTIL_VECTOR_H

#include "anki/util/Assert.h"
#include "anki/util/Allocator.h"
#include <vector>

namespace anki {

/// @addtogroup util_containers
/// @{

/// A semi-custom implementation of vector
template<typename T, typename Alloc = std::allocator<T>>
class Vector: public std::vector<T, Alloc>
{
public:
	typedef std::vector<T, Alloc> Base;

	using Base::Base;
	using Base::operator=;

	typename Base::reference operator[](typename Base::size_type i)
	{
		ANKI_ASSERT(i < Base::size() && "Vector out of bounds");
		return Base::operator[](i);
	}

	typename Base::const_reference operator[](typename Base::size_type i) const
	{
		ANKI_ASSERT(i < Base::size() && "Vector out of bounds");
		return Base::operator[](i);
	}
};

/// @}

} // end namespace anki

#endif
