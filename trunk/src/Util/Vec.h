#ifndef VEC_H
#define VEC_H

#include <vector>
#include "Assert.h"


/// This is a wrapper of std::std::vector that adds new functionality
template<typename Type>
class Vec: public std::vector<Type>
{
	public:
		Vec();
		Vec(size_t size);
		Vec(size_t size, Type val);
		~Vec() {std::vector<Type>::clear();}
		Type& operator[](size_t n);
		const Type& operator[](size_t n) const;
		size_t getSizeInBytes() const;
};


//======================================================================================================================
// Constructor []                                                                                                      =
//======================================================================================================================
template<typename Type>
Vec<Type>::Vec():
	std::vector<Type>()
{}


//======================================================================================================================
// Constructor [size]                                                                                                  =
//======================================================================================================================
template<typename Type>
Vec<Type>::Vec(size_t size):
	std::vector<Type>(size)
{}


//======================================================================================================================
// Constructor [size, val]                                                                                             =
//======================================================================================================================
template<typename Type>
Vec<Type>::Vec(size_t size, Type val):
	std::vector<Type>(size,val)
{}


//======================================================================================================================
// operator[]                                                                                                          =
//======================================================================================================================
template<typename Type>
Type& Vec<Type>::operator[](size_t n)
{
	ASSERT(n < std::vector<Type>::size());
	return std::vector<Type>::operator [](n);
}


//======================================================================================================================
// operator[]                                                                                                          =
//======================================================================================================================
template<typename Type>
const Type& Vec<Type>::operator[](size_t n) const
{
	ASSERT(n < std::vector<Type>::size());
	return std::vector<Type>::operator [](n);
}


//======================================================================================================================
// getSizeInBytes                                                                                                      =
//======================================================================================================================
template<typename Type>
size_t Vec<Type>::getSizeInBytes() const
{
	return std::vector<Type>::size() * sizeof(Type);
}


#endif
