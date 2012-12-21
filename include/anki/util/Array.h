#ifndef ANKI_PtrSizeTIL_ARRAY_H
#define ANKI_PtrSizeTIL_ARRAY_H

#include "anki/util/Assert.h"
#include "anki/util/StdTypes.h"

namespace anki {

/// @addtogroup util
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

	static constexpr PtrSize getSize()
	{
		return N;
	}
};
/// @}

} // end namespace anki

#endif
