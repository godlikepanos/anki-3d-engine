#ifndef RSRC_PTR_H
#define RSRC_PTR_H

#include "Common.h"


/**
 * This is a special smart pointer that points to Resource derivatives. It looks like auto_ptr but the main difference
 * is that when its out of scope it tries to unload the resource.
 */
template<typename Type>
class RsrcPtr
{
	public:
		RsrcPtr();

		/**
		 * It unloads the resource or it decreases its reference counter
		 */
		~RsrcPtr();

		/**
		 * Loads a resource and sets the RsrcPtr::p. The implementation of the function is different for every Resource (see
		 * RsrcPtr.cpp)
		 * @param filename
		 * @return True on success
		 */
		bool loadRsrc(const char* filename);

		Type& operator*() const;
		Type* operator->() const;
		Type* get() const;

	private:
		/**
		 * Non copyable
		 */
		RsrcPtr(const RsrcPtr& a) {}

		/**
		 * Unloads the resource @see loadRsrc
		 */
		void unload();

		Type* p; ///< Pointer to the Resource derivative
};


//======================================================================================================================
// Inlines                                                                                                             =
//======================================================================================================================


template<typename Type>
RsrcPtr<Type>::RsrcPtr():
	p(NULL)
{}


template<typename Type>
RsrcPtr<Type>::~RsrcPtr()
{
	unload();
}


template<typename Type>
Type& RsrcPtr<Type>::operator*() const
{
	DEBUG_ERR(p == NULL);
	return *p;
}


template<typename Type>
Type* RsrcPtr<Type>::operator->() const
{
	DEBUG_ERR(p == NULL);
	return p;
}


template<typename Type>
Type* RsrcPtr<Type>::get() const
{
	return p;
}

#endif
