#ifndef RSRC_PTR_H
#define RSRC_PTR_H

#include "ResourceManager.h"


/// This is a special smart pointer that points to Resource derivatives. It
/// looks like auto_ptr but the main difference is that when its out of scope
/// it tries to unload the resource.
template<typename Type>
class RsrcPtr
{
	public:
		/// Default constructor
		RsrcPtr(): hook(NULL) {}

		/// Copy constructor
		RsrcPtr(const RsrcPtr<Type>& a);

		~RsrcPtr();

		/// Loads a resource and sets the RsrcPtr::p. The implementation of the
		/// function is different for every Resource (see RsrcPtr.cpp)
		/// @param filename
		/// @return True on success
		void loadRsrc(const char* filename);

		Type& operator*() const;
		Type* operator->() const;
		Type* get() const {return hook->resource;}
		const std::string& getRsrcName() const;

	private:
		RsrcHook<Type>* hook; ///< Points to a container in the resource manager

		/// Unloads the resource @see loadRsrc
		void unload();
};


#include "RsrcPtr.inl.h"


#endif
