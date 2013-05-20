#ifndef ANKI_PtrSizeTIL_ARRAY_H
#define ANKI_PtrSizeTIL_ARRAY_H

#include "anki/util/Assert.h"
#include "anki/util/StdTypes.h"

namespace anki {

/// @addtogroup util
/// @{
/// @addtogroup containers
/// @{

/// Like std::array but with some additions
template<typename T, PtrSize N>
struct Array
{
	typedef T Value;
	typedef Value* Iterator;
	typedef const Value* ConstIterator;
	typedef Value& Reference;
	typedef const Value& ConstReference;

	// std compatible
	typedef Iterator iterator;
	typedef ConstIterator const_iterator;
	typedef Reference reference;
	typedef ConstReference const_reference;

	Value data[N];

	Reference operator[](const PtrSize n)
	{
		ANKI_ASSERT(n < N);
		return data[n];
	}

	ConstReference operator[](const PtrSize n) const
	{
		ANKI_ASSERT(n < N);
		return data[n];
	}

	/// Make it compatible with the C++11 range based for loop
	Iterator begin()
	{
		return &data[0];
	}

	/// Make it compatible with the C++11 range based for loop
	ConstIterator begin() const
	{
		return &data[0];
	}

	/// Make it compatible with the C++11 range based for loop
	Iterator end()
	{
		return &data[0] + N;
	}

	/// Make it compatible with the C++11 range based for loop
	ConstIterator end() const
	{
		return &data[0] + N;
	}

	/// Make it compatible with STL
	Reference front() 
	{
		return data[0];
	}

	/// Make it compatible with STL
	ConstReference front() const
	{
		return data[0];
	}

	/// Make it compatible with STL
	Reference back() 
	{
		return data[N - 1];
	}

	/// Make it compatible with STL
	ConstReference back() const
	{
		return data[N - 1];
	}

	static constexpr PtrSize getSize()
	{
		return N;
	}

	/// Make it compatible with STL
	static constexpr PtrSize size()
	{
		return N;
	}
};
/// @}
/// @}

} // end namespace anki

#endif
