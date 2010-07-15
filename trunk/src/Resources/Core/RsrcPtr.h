#ifndef RSRCPTR_H
#define RSRCPTR_H

#include <boost/type_traits.hpp>
#include "Common.h"
#include "RsrcMngr.h"


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
		 * @todo
		 */
		RsrcPtr();

		/**
		 * It unloads the resource or it decreases its reference counter
		 */
		~RsrcPtr();

		/**
		 * @todo
		 * @param filename
		 * @return
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
	if(p != NULL)
	{
		///@todo
	}
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
bool RsrcPtr<Type>::loadRsrc(const char* filename)
{
	if(is_same<Type, Texture>::value)
	{
		p = reinterpret_cast<Type*>(RsrcMngr::textures.load(filename));
	}
	else if(is_same<Type, ShaderProg>::value)
	{
		p = reinterpret_cast<Type*>(RsrcMngr::shaders.load(filename));
	}
	else if(is_same<Type, Material>::value)
	{
		p = reinterpret_cast<Type*>(RsrcMngr::materials.load(filename));
	}
	else if(is_same<Type, Mesh>::value)
	{
		p = reinterpret_cast<Type*>(RsrcMngr::meshes.load(filename));
	}
	else if(is_same<Type, Skeleton>::value)
	{
		p = reinterpret_cast<Type*>(RsrcMngr::skeletons.load(filename));
	}
	else if(is_same<Type, SkelAnim>::value)
	{
		p = reinterpret_cast<Type*>(RsrcMngr::skelAnims.load(filename));
	}
	else if(is_same<Type, LightProps>::value)
	{
		p = reinterpret_cast<Type*>(RsrcMngr::lightProps.load(filename));
	}
	/*else if(is_same<Type, Extension>::value)
	{

	}*/
	else
	{
		FATAL("Unsupported class");
	}

	return p != NULL;
}

#endif
