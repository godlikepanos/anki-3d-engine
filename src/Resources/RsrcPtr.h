#ifndef RSRCPTR_H
#define RSRCPTR_H

#include "Common.h"
#include "Resource.h"


template <typename T>
class ResourceContainer;


/**
 * @todo
 */
template<typename Type>
class RsrcPtr
{
	friend class ResourceContainer<Type>;

	public:
		RsrcPtr();

		/**
		 * This constructor doesn't transfer ownership just like auto_ptr
		 */
		RsrcPtr(const RsrcPtr& a);

		~RsrcPtr();

		RsrcPtr<Type>& operator=(const RsrcPtr<Type>& a);
		Type& operator*();
		Type* operator->();
		Type* get();

	private:
		/**
		 * This constructor is for resource container only
		 */
		RsrcPtr(Type* p_);

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
Type* RsrcPtr<Type>::operator->()
{
	return p;
}

template<typename Type>
Type* RsrcPtr<Type>::get()
{
	return p;
}

#endif
