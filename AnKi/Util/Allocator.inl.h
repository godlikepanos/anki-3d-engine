// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/Allocator.h>
#include <AnKi/Util/WeakArray.h>

namespace anki {

template<typename T, typename TPool>
template<typename TValue, typename TSize>
void GenericPoolAllocator<T, TPool>::deleteArray(WeakArray<TValue, TSize>& arr)
{
	deleteArray(arr.getBegin(), arr.getSize());
	arr.setArray(nullptr, 0);
}

template<typename T, typename TPool>
template<typename TValue, typename TSize>
void GenericPoolAllocator<T, TPool>::newArray(size_type n, WeakArray<TValue, TSize>& out)
{
	TValue* arr = newArray<TValue>(n);
	ANKI_ASSERT(n < std::numeric_limits<TSize>::max());
	out.setArray(arr, TSize(n));
}

template<typename T, typename TPool>
template<typename TValue, typename TSize>
void GenericPoolAllocator<T, TPool>::newArray(size_type n, const TValue& v, WeakArray<TValue, TSize>& out)
{
	TValue* arr = newArray<TValue>(n, v);
	ANKI_ASSERT(n < std::numeric_limits<TSize>::max());
	out.setArray(arr, TSize(n));
}

} // end namespace anki
