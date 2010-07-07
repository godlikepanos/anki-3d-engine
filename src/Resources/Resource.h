#ifndef _RESOURCE_H_
#define _RESOURCE_H_

#include "Common.h"
#include "Util.h"
#include "ResourceContainer.h"


// forward decls
class Texture;
class Material;
class ShaderProg;
class Mesh;
class Skeleton;
class SkelAnim;
class LightProps;


/**
 * Every class that it is considered a resource should be derived by this one. This step is not necessary because of the
 * ResourceContainer template but ensures that loading will be made by the resource manager and not the class itself
 */
class Resource
{
	friend class ResourceContainer<Texture>;
	friend class ResourceContainer<ShaderProg>;
	friend class ResourceContainer<Material>;
	friend class ResourceContainer<Mesh>;
	friend class ResourceContainer<Skeleton>;
	friend class ResourceContainer<SkelAnim>;
	friend class ResourceContainer<LightProps>;

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
		static ResourceContainer<Texture>    textures;
		static ResourceContainer<ShaderProg> shaders;
		static ResourceContainer<Material>   materials;
		static ResourceContainer<Mesh>       meshes;
		static ResourceContainer<Skeleton>   skeletons;
		static ResourceContainer<SkelAnim>   skelAnims;
		static ResourceContainer<LightProps> lightProps;

		Resource(const ResourceType& type_);
		virtual ~Resource();


	private:
		/**
		 * @param filename The file to load
		 * @return True on success
		 */
		virtual bool load(const char* filename) = 0;

		/**
		 * Dont make it pure virtual because the destructor calls it
		 */
		virtual void unload();
};


inline Resource::Resource(const ResourceType& type_):
	referenceCounter(0),
	type(type_)
{}


inline Resource::~Resource()
{
	DEBUG_ERR(referenceCounter != 0);
}


inline void Resource::unload()
{
	FATAL("You have to reimplement this");
}

#endif
