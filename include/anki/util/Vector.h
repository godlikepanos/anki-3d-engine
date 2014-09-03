// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_UTIL_VECTOR_H
#define ANKI_UTIL_VECTOR_H

#include "anki/util/Assert.h"
#include "anki/util/Allocator.h"
#include <vector>

namespace anki {

/// @addtogroup util_containers
/// @{

/// A semi-custom implementation of vector
template<typename T, template <typename> class TAlloc = HeapAllocator>
class Vector: public std::vector<T, TAlloc<T>>
{
public:
	using Base = std::vector<T, TAlloc<T>>;

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

	PtrSize getSizeInBytes() const
	{
		return Base::size() * sizeof(T);
	}
};

/// @}

} // end namespace anki

#endif
