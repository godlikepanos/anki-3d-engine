#ifndef RESOURCE_H
#define RESOURCE_H

#include "Common.h"
#include "Util.h"


template<typename Type>
class RsrcContainer;


/// Every class that it is considered a resource should be derived by this one. This step is not necessary because of
/// the RsrcContainer template but ensures that loading will be made by the resource manager and not the class itself
class Resource
{
	template<typename Type>
	friend class RsrcContainer; ///< Cause it calls Resource::load and Resource::unload

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
			RT_EXTENSION,
			RT_PARTICLE_EMITTER_PROPS,
			RT_SCRIPT
		};

	PROPERTY_R(string, path, getRsrcPath);
	PROPERTY_R(string, name, getRsrcName);
	PROPERTY_R(uint, referenceCounter, getRsrcReferencesNum);
	PROPERTY_R(ResourceType, type, getRsrcType);

	public:
		Resource(const ResourceType& type_);
		virtual ~Resource() {DEBUG_ERR(referenceCounter != 0);}

	private:
		/// Load the resource
		/// @param filename The file to load
		/// @exception Exception
		virtual void load(const char* filename) = 0;
};


inline Resource::Resource(const ResourceType& type_):
	referenceCounter(0),
	type(type_)
{}


#endif
