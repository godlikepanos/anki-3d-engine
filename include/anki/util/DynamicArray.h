#ifndef ANKI_UTIL_DYNAMIC_ARRAY_H
#define ANKI_UTIL_DYNAMIC_ARRAY_H

#include "anki/util/Allocator.h"

namespace anki {

/// @addtogroup util
/// @{
/// @addtogroup containers
/// @{

template<typename T, typename Alloc = Allocator<T>>
class DynamicArray
{
public:
	typedef T Value;
	typedef Value* Iterator;
	typedef const Value* ConstIterator;
	typedef Value& Reference;
	typedef const Value& ConstReference;

	/// @name Constructors/destructor
	/// @{

	/// XXX
	DynamicArray()
	{}
	/// XXX
	DynamicArray(const PtrSize n)
	{

	}
	/// @}

	Reference operator[](const PtrSize n)
	{
		ANKI_ASSERT(n < getSize());
		return *(begin + n);
	}

	ConstReference operator[](const PtrSize n) const
	{
		ANKI_ASSERT(n < getSize());
		return *(begin + n);
	}

	/// Make it compatible with the C++11 range based for loop
	Iterator begin()
	{
		return b;
	}

	/// Make it compatible with the C++11 range based for loop
	ConstIterator begin() const
	{
		return b;
	}

	/// Make it compatible with the C++11 range based for loop
	Iterator end()
	{
		return e;
	}

	/// Make it compatible with the C++11 range based for loop
	ConstIterator end() const
	{
		return e;
	}

	PtrSize getSize() const
	{
		return e - b;
	}

private:
	Value* b = nullptr;
	Value* e = nullptr;
	Alloc allocator;
};

/// @}
/// @}

} // end namespace anki

#endif
