#ifndef RESOURCECONTAINER_H
#define RESOURCECONTAINER_H

#include "Common.h"
#include "Vec.h"
#include "RsrcPtr.h"


/**
 * Resource container template class
 */
template<typename Type>
class RsrcContainer: public Vec<Type*>
{
	friend class Resource;

	private:
		typedef Vec<Type*> BaseClass;
		typedef typename BaseClass::iterator Iterator; ///< Just to save me time from typing

	public:
		RsrcContainer() {}
		~RsrcContainer();

		/**
		 * The one and only public func
		 * @param fname The file to load
		 * @return A new resource ptr
		 */
		RsrcPtr<Type> load(const char* fname);

	private:
		/**
		 * load an object and register it. If its already loaded return its pointer
		 * @param fname The filename that initializes the object
		 * @return A pointer of a new resource or NULL on fail
		 */
		Type* load2(const char* fname);

		/**
		 * unload item. If nobody else uses it then delete it completely
		 * @param x Pointer to the instance we want to unload
		 */
		void unload(Type* x);

		/**
		 * Search inside the container by name
		 * @param name The name of the resource
		 * @return The iterator of the content end of vector if not found
		 */
		Iterator findByName(const char* name);

		/**
		 * Search inside the container by name and path
		 * @param name The name of the resource
		 * @param path The path of the resource
		 * @return The iterator of the content end of vector if not found
		 */
		Iterator findByNameAndPath(const char* name, const char* path);

		/**
	   * Search inside the container by pointer
		 * @param name The name of the resource object
		 * @return The iterator of the content end of vector if not found
		 */
		Iterator findByPtr(Type* ptr);
};


#include "RsrcContainer.inl.h"

#endif
