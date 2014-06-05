// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_PtrSizeTIL_ARRAY_H
#define ANKI_PtrSizeTIL_ARRAY_H

#include "anki/util/Assert.h"
#include "anki/util/StdTypes.h"

namespace anki {

/// @addtogroup util_containers
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

	Value m_data[N];

	Reference operator[](const PtrSize n)
	{
		ANKI_ASSERT(n < N);
		return m_data[n];
	}

	ConstReference operator[](const PtrSize n) const
	{
		ANKI_ASSERT(n < N);
		return m_data[n];
	}

	/// Make it compatible with the C++11 range based for loop
	Iterator begin()
	{
		return &m_data[0];
	}

	/// Make it compatible with the C++11 range based for loop
	ConstIterator begin() const
	{
		return &m_data[0];
	}

	/// Make it compatible with the C++11 range based for loop
	Iterator end()
	{
		return &m_data[0] + N;
	}

	/// Make it compatible with the C++11 range based for loop
	ConstIterator end() const
	{
		return &m_data[0] + N;
	}

	/// Make it compatible with STL
	Reference front() 
	{
		return m_data[0];
	}

	/// Make it compatible with STL
	ConstReference front() const
	{
		return m_data[0];
	}

	/// Make it compatible with STL
	Reference back() 
	{
		return m_data[N - 1];
	}

	/// Make it compatible with STL
	ConstReference back() const
	{
		return m_data[N - 1];
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

/// 2D Array. @code Array2d<X, 10, 2> a; @endcode is equivelent to 
/// @code X a[10][2]; @endcode
template<typename T, PtrSize I, PtrSize J>
using Array2d = Array<Array<T, J>, I>;

/// 3D Array. @code Array3d<X, 10, 2, 3> a; @endcode is equivelent to 
/// @code X a[10][2][3]; @endcode
template<typename T, PtrSize I, PtrSize J, PtrSize K>
using Array3d = Array<Array<Array<T, K>, J>, I>;

/// @}

} // end namespace anki

#endif
