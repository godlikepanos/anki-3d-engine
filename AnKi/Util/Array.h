// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/Functions.h>

namespace anki {

/// @addtogroup util_containers
/// @{

/// Like std::array but with some additions
template<typename T, PtrSize kSize>
class Array
{
public:
	using Value = T;
	using Iterator = Value*;
	using ConstIterator = const Value*;
	using Reference = Value&;
	using ConstReference = const Value&;

	// STL compatible
	using iterator = Iterator;
	using const_iterator = ConstIterator;
	using reference = Reference;
	using const_reference = ConstReference;

	Value m_data[kSize];

	/// Access an element using an integer.
	template<typename TInt, ANKI_ENABLE(!std::is_enum<TInt>::value)>
	Reference operator[](const TInt n)
	{
		ANKI_ASSERT(PtrSize(n) < kSize);
		return m_data[n];
	}

	/// Access an element using an integer.
	template<typename TInt, ANKI_ENABLE(!std::is_enum<TInt>::value)>
	ConstReference operator[](const TInt n) const
	{
		ANKI_ASSERT(PtrSize(n) < kSize);
		return m_data[n];
	}

	/// Access an element using an enumerant. It's a little bit special and separate from operator[] that accepts
	/// integer. This to avoid any short of arbitrary integer type casting.
	template<typename TEnum, ANKI_ENABLE(std::is_enum<TEnum>::value)>
	Reference operator[](const TEnum n)
	{
		return operator[](typename std::underlying_type<TEnum>::type(n));
	}

	/// Access an element using an enumerant. It's a little bit special and separate from operator[] that accepts
	/// integer. This to avoid any short of arbitrary integer type casting.
	template<typename TEnum, ANKI_ENABLE(std::is_enum<TEnum>::value)>
	ConstReference operator[](const TEnum n) const
	{
		return operator[](typename std::underlying_type<TEnum>::type(n));
	}

	Iterator getBegin()
	{
		return &m_data[0];
	}

	ConstIterator getBegin() const
	{
		return &m_data[0];
	}

	Iterator getEnd()
	{
		return &m_data[0] + kSize;
	}

	ConstIterator getEnd() const
	{
		return &m_data[0] + kSize;
	}

	Reference getFront()
	{
		return m_data[0];
	}

	ConstReference getFront() const
	{
		return m_data[0];
	}

	Reference getBack()
	{
		return m_data[kSize - 1];
	}

	ConstReference getBack() const
	{
		return m_data[kSize - 1];
	}

	/// Make it compatible with the C++11 range based for loop
	Iterator begin()
	{
		return getBegin();
	}

	/// Make it compatible with the C++11 range based for loop
	ConstIterator begin() const
	{
		return getBegin();
	}

	/// Make it compatible with the C++11 range based for loop
	Iterator end()
	{
		return getEnd();
	}

	/// Make it compatible with the C++11 range based for loop
	ConstIterator end() const
	{
		return getEnd();
	}

	/// Make it compatible with STL
	Reference front()
	{
		return getFront();
	}

	/// Make it compatible with STL
	ConstReference front() const
	{
		return getFront();
	}

	/// Make it compatible with STL
	Reference back()
	{
		return getBack;
	}

	/// Make it compatible with STL
	ConstReference back() const
	{
		return getBack();
	}

	// Get size
#define ANKI_ARRAY_SIZE_METHOD(type, condition) \
	ANKI_ENABLE_METHOD(condition) \
	static constexpr type getSize() \
	{ \
		return type(kSize); \
	}
	ANKI_ARRAY_SIZE_METHOD(U8, kSize <= kMaxU8)
	ANKI_ARRAY_SIZE_METHOD(U16, kSize > kMaxU8 && kSize <= kMaxU16)
	ANKI_ARRAY_SIZE_METHOD(U32, kSize > kMaxU16 && kSize <= kMaxU32)
	ANKI_ARRAY_SIZE_METHOD(U64, kSize > kMaxU32)
#undef ANKI_ARRAY_SIZE_METHOD

	/// Make it compatible with STL
	static constexpr size_t size()
	{
		return kSize;
	}

	/// Fill the array.
	void fill(Iterator begin, Iterator end, const T& val)
	{
		while(begin != end)
		{
			*begin = val;
			++begin;
		}
	}

	// Get size in bytes
#define ANKI_ARRAY_SIZE_IN_BYTES_METHOD(type, condition) \
	ANKI_ENABLE_METHOD(condition) \
	static constexpr type getSizeInBytes() \
	{ \
		return type(kSize * sizeof(Value)); \
	}
	ANKI_ARRAY_SIZE_IN_BYTES_METHOD(U8, kSize * sizeof(Value) <= kMaxU8)
	ANKI_ARRAY_SIZE_IN_BYTES_METHOD(U16, kSize * sizeof(Value) > kMaxU8 && kSize * sizeof(Value) <= kMaxU16)
	ANKI_ARRAY_SIZE_IN_BYTES_METHOD(U32, kSize * sizeof(Value) > kMaxU16 && kSize * sizeof(Value) <= kMaxU32)
	ANKI_ARRAY_SIZE_IN_BYTES_METHOD(U64, kSize * sizeof(Value) > kMaxU32)
#undef ANKI_ARRAY_SIZE_IN_BYTES_METHOD
};

// Some trick stolen from GCC's std::array. It allows deduction of Array's template params. For example you can write:
// Array a{1, 2, 3};
template<typename TFirst, typename... TRest>
Array(TFirst, TRest...)
	-> Array<std::enable_if_t<(std::is_same_v<TFirst, TRest> && ...), TFirst>, 1 + sizeof...(TRest)>;

/// 2D Array. @code Array2d<X, 10, 2> a; @endcode is equivelent to @code X a[10][2]; @endcode
template<typename T, PtrSize I, PtrSize J>
using Array2d = Array<Array<T, J>, I>;

/// 3D Array. @code Array3d<X, 10, 2, 3> a; @endcode is equivelent to @code X a[10][2][3]; @endcode
template<typename T, PtrSize I, PtrSize J, PtrSize K>
using Array3d = Array<Array<Array<T, K>, J>, I>;

/// 4D Array. @code Array4d<X, 10, 2, 3, 4> a; @endcode is equivelent to @code X a[10][2][3][4]; @endcode
template<typename T, PtrSize I, PtrSize J, PtrSize K, PtrSize L>
using Array4d = Array<Array<Array<Array<T, L>, K>, J>, I>;

/// 5D Array. @code Array5d<X, 10, 2, 3, 4, 5> a; @endcode is equivelent to @code X a[10][2][3][4][5]; @endcode
template<typename T, PtrSize I, PtrSize J, PtrSize K, PtrSize L, PtrSize M>
using Array5d = Array<Array<Array<Array<Array<T, M>, L>, K>, J>, I>;
/// @}

} // end namespace anki
