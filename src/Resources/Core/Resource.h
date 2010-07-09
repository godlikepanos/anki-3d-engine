#ifndef _RESOURCE_H_
#define _RESOURCE_H_

#include "Common.h"
#include "Util.h"
#include "RsrcContainer.h"


template<typename Type>
class Rsrc;


/**
 * Every class that it is considered a resource should be derived by this one. This step is not necessary because of the
 * RsrcContainer template but ensures that loading will be made by the resource manager and not the class itself
 */
class Resource
{
	template<typename Type>
	friend class RsrcContainer;

	// to be able to call tryToUnoadMe
	template<typename Type>
	friend class RsrcPtr;

	public:
		enum ResourceType
		{
			RT_TEXTURE,
			RT_SHADER_PROG,
			RT_MATERIAL,
			RT_MESH,
			RT_SKELETON,
			RT_SKEL_ANIM,
			RT_LIGHT_PROPS,
			RT_EXTENSION
		};

	PROPERTY_R(string, path, getRsrcPath);
	PROPERTY_R(string, name, getRsrcName);
	PROPERTY_R(uint, referenceCounter, getRsrcReferencesNum);
	PROPERTY_R(ResourceType, type, getRsrcType);

	public:
		Resource(const ResourceType& type_);
		virtual ~Resource();

	private:
		/**
		 * @param filename The file to load
		 * @return True on success
		 */
		virtual bool load(const char* filename) = 0;

		virtual void unload() = 0;

		/**
		 * The func sees the resource type and calls the unload func of the appropriate container. Used by RsrcPtr
		 */
		void tryToUnoadMe();
};


inline Resource::Resource(const ResourceType& type_):
	referenceCounter(0),
	type(type_)
{}


inline Resource::~Resource()
{
	DEBUG_ERR(referenceCounter != 0);
}

#endif
