#ifndef ANKI_UTIL_VECTOR_H
#define ANKI_UTIL_VECTOR_H

#include "anki/util/Assert.h"
#include <vector>
#include <cassert>

namespace anki {

/// @addtogroup util
/// @{

template<typename T>
using Vector = std::vector<T>;

#if 0
/// This is a wrapper of Vector that adds new functionality and assertions
/// in operator[]
template<typename Type, typename Allocator = std::allocator<Type>>
class Vector: public std::vector<Type, Allocator>
{
public:
	typedef Vector<Type, Allocator> Base;
	typedef typename Base::value_type value_type;
	typedef typename Base::allocator_type allocator_type;
	typedef typename Base::size_type size_type;
	typedef typename Base::reference reference;
	typedef typename Base::const_reference const_reference;
	typedef typename Base::iterator iterator;
	typedef typename Base::const_iterator const_iterator;
	typedef typename Base::pointer pointer;

	/// @name Constructors & destructor
	/// @{
	Vector()
		: Base()
	{}

	Vector(const allocator_type& a)
		: Base(a)
	{}

	Vector(size_type n,
		const value_type& value = value_type(),
		const allocator_type& a = allocator_type())
		: Base(n, value, a)
	{}

	Vector(const Vector& x)
		: Base(x)
	{}

	template<typename InputIterator>
	Vector(InputIterator first, InputIterator last,
		const allocator_type& a = allocator_type())
		: Base(first, last, a)
	{}
	/// @}

	/// @name Accessors
	/// @{
	reference operator[](size_type n)
	{
		ANKI_ASSERT(n < Base::size());
		return Base::operator[](n);
	}

	const_reference operator[](size_type n) const
	{
		ANKI_ASSERT(n < Base::size());
		return Base::operator[](n);
	}
	/// @}
};
#endif
/// @}

} // end namespace anki

#endif
