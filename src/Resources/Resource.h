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
	friend class ShaderProg;

	PROPERTY_R(string, path, getRsrcPath);
	PROPERTY_R(string, name, getRsrcName);
	PROPERTY_R(uint, usersNum, getRsrcUsersNum);

	public:
		static ResourceContainer<Texture>    textures;
		static ResourceContainer<ShaderProg> shaders;
		static ResourceContainer<Material>   materials;
		static ResourceContainer<Mesh>       meshes;
		static ResourceContainer<Skeleton>   skeletons;
		static ResourceContainer<SkelAnim>   skelAnims;
		static ResourceContainer<LightProps> lightProps;

		/**
		 * @param filename The file to load
		 * @return True on success
		 */
		virtual bool load(const char* filename) = 0;
		virtual void unload() = 0;

		Resource(): usersNum(0) {}
		virtual ~Resource() {};
};
#endif
