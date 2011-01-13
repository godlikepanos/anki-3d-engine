#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <list>
#include <string>
#include "Singleton.h"


class Texture;
class ShaderProg;
class Material;
class Mesh;
class Skeleton;
class SkelAnim;
class LightData;
class ParticleEmitterProps;
class Script;
class Model;
class Skin;
class DummyRsrc;


/// Includes information about the resources
template<typename Type>
struct RsrcHook
{
	std::string uuid;
	int referenceCounter;
	Type* resource;
};


/// Responsible of loading and unloading resources
class ResourceManager
{
	public:
		/// Because the current C++ standard does not offer template typedefs
		template<typename Type>
		struct Types
		{
			typedef RsrcHook<Type> Hook;
			typedef std::list<Hook> Container;
			typedef typename Container::iterator Iterator;
			typedef typename Container::const_iterator ConstIterator;
		};

		/// Load a resource
		/// See if its already loaded, if its not:
		/// - Create a instance
		/// - Call load method of the instance
		/// If its loaded:
		/// - Increase the resource counter
		template<typename Type>
		typename Types<Type>::Hook& load(const char* filename);

		/// Unload a resource if no one uses it
		template<typename Type>
		void unload(const typename Types<Type>::Hook& info);

	private:
		/// Find a resource using the filename
		template<typename Type>
		typename Types<Type>::Iterator find(const char* filename, typename Types<Type>::Container& container);

		/// Find a resource using the pointer
		template<typename Type>
		typename Types<Type>::Iterator find(const Type* resource, typename Types<Type>::Container& container);

		/// Specialized func
		template<typename Type>
		typename Types<Type>::Container& choseContainer();

		/// Unload a resource if no one uses it. This is the real deal
		template<typename Type>
		void unloadR(const typename Types<Type>::Hook& info);

		/// @name Containers
		/// @{
		Types<Texture>::Container textures;
		Types<ShaderProg>::Container shaderProgs;
		Types<Material>::Container materials;
		Types<Mesh>::Container meshes;
		Types<Skeleton>::Container skeletons;
		Types<SkelAnim>::Container skelAnims;
		Types<LightData>::Container lightProps;
		Types<ParticleEmitterProps>::Container particleEmitterProps;
		Types<Script>::Container scripts;
		Types<Model>::Container models;
		Types<Skin>::Container skins;
		Types<DummyRsrc>::Container dummies;
		/// @}
};


#include "ResourceManager.inl.h"


typedef Singleton<ResourceManager> ResourceManagerSingleton;


#endif
