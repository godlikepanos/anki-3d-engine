#ifndef RSRC_PTR_H
#define RSRC_PTR_H

#include "Exception.h"

#ifndef NULL
	#define NULL 0
#endif


/// This is a special smart pointer that points to Resource derivatives. It looks like auto_ptr but the main difference
/// is that when its out of scope it tries to unload the resource.
template<typename Type>
class RsrcPtr
{
	public:
		/// Default constructor
		RsrcPtr(): p(NULL) {}

		/// Copy constructor
		RsrcPtr(const RsrcPtr& a);

		/// It unloads the resource or it decreases its reference counter
		~RsrcPtr() {unload();}

		/// Loads a resource and sets the RsrcPtr::p. The implementation of the function is different for every Resource
		/// (see RsrcPtr.cpp)
		/// @param filename
		/// @return True on success
		bool loadRsrc(const char* filename);

		Type& operator*() const;
		Type* operator->() const;
		Type* get() const {return p;}

	private:
		/// Unloads the resource @see loadRsrc
		void unload();

		Type* p; ///< Pointer to the Resource derivative
};


//======================================================================================================================
// Inlines                                                                                                             =
//======================================================================================================================

template<typename Type>
RsrcPtr<Type>::RsrcPtr(const RsrcPtr& a):
	p(a.p)
{
	if(p)
	{
		++p->referenceCounter;
	}
}


template<typename Type>
Type& RsrcPtr<Type>::operator*() const
{
	RASSERT_THROW_EXCEPTION(p == NULL);
	return *p;
}


template<typename Type>
Type* RsrcPtr<Type>::operator->() const
{
	RASSERT_THROW_EXCEPTION(p == NULL);
	return p;
}


#endif
