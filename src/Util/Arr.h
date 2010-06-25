#ifndef ARR_H
#define ARR_H

#include <boost/array.hpp>
#include "Common.h"


/**
 * This is a wrapper of boost::array that adds new functionality
 */
template<typename Type, size_t nn>
class Arr: public boost::array<Type, nn>
{
	public:
		Type& operator[](size_t n);
		const Type& operator[](size_t n) const;
};


//======================================================================================================================
// operator[]                                                                                                          =
//======================================================================================================================
template<typename Type, size_t nn>
Type& Arr<Type, nn>::operator[](size_t n)
{
	DEBUG_ERR(n >= nn);
	return boost::array<Type, nn>::operator [](n);
}


//======================================================================================================================
// operator[]                                                                                                          =
//======================================================================================================================
template<typename Type, size_t nn>
const Type& Arr<Type, nn>::operator[](size_t n) const
{
	DEBUG_ERR(n >= nn);
	return boost::array<Type, nn>::operator [](n);
}

#endif
