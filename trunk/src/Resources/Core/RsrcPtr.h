#ifndef RSRCPTR_H
#define RSRCPTR_H

#include "Common.h"


/**
 * This is a special pointer to a resource. Its not smart or anything, the only difference is that when its out of scope
 * it tries to unload the resource
 */
template<typename Type>
class RsrcPtr
{
	public:
		/**
		 * This constructor transfers ownership just like auto_ptr
		 */
		template<typename Type1>
		RsrcPtr(RsrcPtr<Type1>& a);

		/**
		 * This constructor is for resource container only
		 */
		explicit RsrcPtr(Type* p_ = NULL);

		~RsrcPtr();


		template<typename Type1>
		RsrcPtr<Type1>& operator=(RsrcPtr<Type1>& a);

		Type& operator*();
		const Type& operator*() const;
		Type* operator->();
		const Type* operator->() const;
		Type* get();
		const Type* get() const;

	private:
		Type* p;
};


//======================================================================================================================
// Inlines                                                                                                             =
//======================================================================================================================

template<typename Type>
template<typename Type1>
RsrcPtr<Type>::RsrcPtr(RsrcPtr<Type1>& a)
{
	p = a.p;
	a.p = NULL;
}


template<typename Type>
RsrcPtr<Type>::RsrcPtr(Type* p_):
	p(p_)
{}


template<typename Type>
RsrcPtr<Type>::~RsrcPtr()
{
	if(p != NULL)
		p->tryToUnoadMe();
}


template<typename Type>
template<typename Type1>
RsrcPtr<Type1>& RsrcPtr<Type>::operator=(RsrcPtr<Type1>& a)
{
	DEBUG_ERR(p != NULL);
	p = a.p;
	a.p = NULL;
	return *this;
}


template<typename Type>
Type& RsrcPtr<Type>::operator*()
{
	return *p;
}


template<typename Type>
const Type& RsrcPtr<Type>::operator*() const
{
	return *p;
}


template<typename Type>
Type* RsrcPtr<Type>::operator->()
{
	return p;
}


template<typename Type>
const Type* RsrcPtr<Type>::operator->() const
{
	return p;
}


template<typename Type>
Type* RsrcPtr<Type>::get()
{
	return p;
}


template<typename Type>
const Type* RsrcPtr<Type>::get() const
{
	return p;
}

#endif
