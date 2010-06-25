#ifndef VEC_H
#define VEC_H

#include <vector>
#include "Common.h"


/**
 * This is a wrapper of std::vector that adds new functionality
 */
template<typename Type>
class Vec: public vector<Type>
{
	public:
		Vec();
		Vec(size_t size);
		Vec(size_t size, Type val);
		Type& operator[](size_t n);
		const Type& operator[](size_t n) const;
		size_t getSizeInBytes() const;
};


//======================================================================================================================
// Constructor []                                                                                                      =
//======================================================================================================================
template<typename Type>
Vec<Type>::Vec():
	vector<Type>()
{}


//======================================================================================================================
// Constructor [size]                                                                                                  =
//======================================================================================================================
template<typename Type>
Vec<Type>::Vec(size_t size):
	vector<Type>(size)
{}


//======================================================================================================================
// Constructor [size, val]                                                                                             =
//======================================================================================================================
template<typename Type>
Vec<Type>::Vec(size_t size, Type val):
	vector<Type>(size,val)
{}


//======================================================================================================================
// operator[]                                                                                                          =
//======================================================================================================================
template<typename Type>
Type& Vec<Type>::operator[](size_t n)
{
	DEBUG_ERR(n >= vector<Type>::size());
	return vector<Type>::operator [](n);
}


//======================================================================================================================
// operator[]                                                                                                          =
//======================================================================================================================
template<typename Type>
const Type& Vec<Type>::operator[](size_t n) const
{
	DEBUG_ERR(n >= vector<Type>::size());
	return vector<Type>::operator [](n);
}


//======================================================================================================================
// getSizeInBytes                                                                                                      =
//======================================================================================================================
template<typename Type>
size_t Vec<Type>::getSizeInBytes() const
{
	return vector<Type>::size() * sizeof(Type);
}

#endif
