#ifndef RSRCPTR_H
#define RSRCPTR_H

#include "Common.h"


/**
 * This is a special smart pointer that points to Resource derivatives. It looks like auto_ptr but the main difference
 * is that when its out of scope it tries to unload the resource. The bad thing about this pointer it contains an ugly
 * hack. Without this hack we should have build our own version of auto_ptr
 */
template<typename Type>
class RsrcPtr
{
	public:
		/**
		 * This constructor transfers ownership just like auto_ptr despite the const reference (hack)
		 */
		RsrcPtr(const RsrcPtr& a);

		/**
		 * This constructor is for resource container only
		 */
		explicit RsrcPtr(Type* p_ = NULL);

		/**
		 * It unloads the resource or it decreases its reference counter
		 */
		~RsrcPtr();

		Type* release();

		/**
		 * It transfers ownership despite the const reference (hack)
		 */
		RsrcPtr& operator=(const RsrcPtr& a);

		Type& operator*() const;
		Type* operator->() const;
		Type* get() const;

	private:
		Type* p;
};


//======================================================================================================================
// Inlines                                                                                                             =
//======================================================================================================================

template<typename Type>
RsrcPtr<Type>::RsrcPtr(const RsrcPtr& a):
	p(const_cast<RsrcPtr&>(a).release())
{}


template<typename Type>
RsrcPtr<Type>::RsrcPtr(Type* p_):
	p(p_)
{}


template<typename Type>
RsrcPtr<Type>::~RsrcPtr()
{
	if(p != NULL)
	{
		p->tryToUnoadMe();
		release();
	}
}


template<typename Type>
RsrcPtr<Type>& RsrcPtr<Type>::operator=(const RsrcPtr<Type>& a)
{
	DEBUG_ERR(p != NULL);
	p = const_cast<RsrcPtr&>(a).release();
	return *this;
}


template<typename Type>
Type& RsrcPtr<Type>::operator*() const
{
	return *p;
}


template<typename Type>
Type* RsrcPtr<Type>::operator->() const
{
	return p;
}


template<typename Type>
Type* RsrcPtr<Type>::get() const
{
	return p;
}


template<typename Type>
Type* RsrcPtr<Type>::release()
{
	Type* p_ = p;
	p = NULL;
	return p_;
}

#endif
