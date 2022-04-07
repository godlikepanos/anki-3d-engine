// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/WeakArray.h>
#include <AnKi/Util/DynamicArray.h>

namespace anki {

template<typename T, typename TSize>
WeakArray<T, TSize>::WeakArray(DynamicArray<T, TSize>& arr)
	: WeakArray()
{
	if(arr.getSize())
	{
		m_data = &arr[0];
		m_size = arr.getSize();
	}
}

template<typename T, typename TSize>
WeakArray<T, TSize>::WeakArray(DynamicArrayAuto<T, TSize>& arr)
	: WeakArray()
{
	if(arr.getSize())
	{
		m_data = &arr[0];
		m_size = arr.getSize();
	}
}

template<typename T, typename TSize>
WeakArray<T, TSize>& WeakArray<T, TSize>::operator=(DynamicArray<T, TSize>& arr)
{
	m_data = (arr.getSize()) ? &arr[0] : nullptr;
	m_size = arr.getSize();
	return *this;
}

template<typename T, typename TSize>
WeakArray<T, TSize>& WeakArray<T, TSize>::operator=(DynamicArrayAuto<T, TSize>& arr)
{
	m_data = (arr.getSize()) ? &arr[0] : nullptr;
	m_size = arr.getSize();
	return *this;
}

template<typename T, typename TSize>
ConstWeakArray<T, TSize>::ConstWeakArray(const DynamicArray<T, TSize>& arr)
	: ConstWeakArray()
{
	if(arr.getSize())
	{
		m_data = &arr[0];
		m_size = arr.getSize();
	}
}

template<typename T, typename TSize>
ConstWeakArray<T, TSize>::ConstWeakArray(const DynamicArrayAuto<T, TSize>& arr)
	: ConstWeakArray()
{
	if(arr.getSize())
	{
		m_data = &arr[0];
		m_size = arr.getSize();
	}
}

template<typename T, typename TSize>
ConstWeakArray<T, TSize>& ConstWeakArray<T, TSize>::operator=(const DynamicArray<T, TSize>& arr)
{
	m_data = (arr.getSize()) ? &arr[0] : nullptr;
	m_size = arr.getSize();
	return *this;
}

template<typename T, typename TSize>
ConstWeakArray<T, TSize>& ConstWeakArray<T, TSize>::operator=(const DynamicArrayAuto<T, TSize>& arr)
{
	m_data = (arr.getSize()) ? &arr[0] : nullptr;
	m_size = arr.getSize();
	return *this;
}

} // end namespace anki
