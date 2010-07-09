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
		RsrcPtr();

		/**
		 * This constructor doesn't transfer ownership just like auto_ptr
		 */
		RsrcPtr(const RsrcPtr& a);

		/**
		 * This constructor is for resource container only
		 */
		RsrcPtr(Type* p_);

		~RsrcPtr();

		RsrcPtr<Type>& operator=(const RsrcPtr<Type>& a);
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
RsrcPtr<Type>::RsrcPtr():
	p(NULL)
{}


template<typename Type>
RsrcPtr<Type>::RsrcPtr(const RsrcPtr& a):
	p(a.p)
{
	DEBUG_ERR(a.p == NULL);
}


template<typename Type>
RsrcPtr<Type>::RsrcPtr(Type* p_):
	p(p_)
{}


template<typename Type>
RsrcPtr<Type>::~RsrcPtr()
{
	if(p == NULL) return;
	p->tryToUnoadMe();
}


template<typename Type>
RsrcPtr<Type>& RsrcPtr<Type>::operator=(const RsrcPtr<Type>& a)
{
	DEBUG_ERR(p != NULL);
	p = a.p;
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
