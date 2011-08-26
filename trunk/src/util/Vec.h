#ifndef VEC_H
#define VEC_H

#include <vector>
#include "util/Assert.h"


/// This is a wrapper of std::vector that adds new functionality and assertions in operator[]
template<typename Type, typename Allocator = std::allocator<Type> >
class Vec: public std::vector<Type, Allocator>
{
	public:
		typedef std::vector<Type, Allocator> Base;

		/// @name Constructors/destructors
		/// @{
		Vec(const Allocator& al = Allocator()): Base(al) {}
		Vec(size_t size, const Type& value = Type(), const Allocator& al = Allocator()): Base(size, value, al) {}

		template <typename InputIterator>
		Vec(InputIterator first, InputIterator last, const Allocator& al = Allocator()): Base(first, last, al) {}

		Vec(const Vec<Type, Allocator>& b): Base(b) {}
		/// @}

		/// @name Accessors
		/// @{
		Type& operator[](size_t n);
		const Type& operator[](size_t n) const;
		size_t getSizeInBytes() const;
		/// @}
};


//==============================================================================
// operator[]                                                                  =
//==============================================================================
template<typename Type, typename Allocator>
Type& Vec<Type, Allocator>::operator[](size_t n)
{
	ASSERT(n < Base::size());
	return Base::operator [](n);
}


//==============================================================================
// operator[]                                                                  =
//==============================================================================
template<typename Type, typename Allocator>
const Type& Vec<Type, Allocator>::operator[](size_t n) const
{
	ASSERT(n < Base::size());
	return Base::operator [](n);
}


//==============================================================================
// getSizeInBytes                                                              =
//==============================================================================
template<typename Type, typename Allocator>
size_t Vec<Type, Allocator>::getSizeInBytes() const
{
	return Base::size() * sizeof(Type);
}


#endif
