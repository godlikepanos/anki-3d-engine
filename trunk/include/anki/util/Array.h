#ifndef ANKI_UTIL_ARRAY_H
#define ANKI_UTIL_ARRAY_H

#include "anki/util/Assert.h"
#include "anki/util/StdTypes.h"

namespace anki {

/// @addtogroup util
/// @{

/// Array
template<typename T, U N>
struct Array
{
	typedef T Value;
	typedef Value* Iterator;
	typedef const Value* ConstIterator;
	typedef Value& Reference;
	typedef const Value& ConstReference;

	Value data[N];

	Reference operator[](U n)
	{
		ANKI_ASSERT(n < N);
		return data[n];
	}

	ConstReference operator[](U n) const
	{
		ANKI_ASSERT(n < N);
		return data[n];
	}

	Iterator begin()
	{
		return &data[0];
	}

	ConstIterator begin() const
	{
		return &data[0];
	}

	Iterator end()
	{
		return &data[0] + N;
	}

	ConstIterator end() const
	{
		return &data[0] + N;
	}

	constexpr U getSize() const
	{
		return N;
	}
};
/// @}

} // end namespace anki

#endif
