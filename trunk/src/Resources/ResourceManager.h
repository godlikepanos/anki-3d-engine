#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/scoped_ptr.hpp>
#include <string>
#include "Core/AsyncLoader.h"
#include "Util/Accessors.h"
#include "RsrcHook.h"
#include "RsrcAsyncLoadingReqsHandler.h"


class Texture;
class ShaderProgram;
class Material;
class Mesh;
class Skeleton;
class SkelAnim;
class LightRsrc;
class ParticleEmitterRsrc;
class Script;
class Model;
class Skin;
class DummyRsrc;


/// Responsible of loading and unloading resources
class ResourceManager
{
	public:
		/// Because the current C++ standard does not offer template typedefs
		template<typename Type>
		struct Types
		{
			typedef RsrcHook<Type> Hook;
			typedef boost::ptr_vector<Hook> Container;
			typedef typename Container::iterator Iterator;
			typedef typename Container::const_iterator ConstIterator;
		};

		ResourceManager();
		~ResourceManager();

		/// Load a resource
		/// See if its already loaded, if its not:
		/// - Create an instance
		/// - Call load method of the instance
		/// If its loaded:
		/// - Increase the resource counter
		template<typename Type>
		typename Types<Type>::Hook& load(const char* filename);

		/// Unload a resource if no one uses it
		template<typename Type>
		void unload(const typename Types<Type>::Hook& info);
		
		/// See RsrcAsyncLoadingReqsHandler::postProcessFinishedRequests
		void postProcessFinishedLoadingRequests(float maxTime);

		/// Get the number of total pending requests
		size_t getAsyncLoadingRequestsNum() const;

	private:
		/// @name Containers
		/// @{
		Types<Texture>::Container textures;
		Types<ShaderProgram>::Container shaderProgs;
		Types<Material>::Container materials;
		Types<Mesh>::Container meshes;
		Types<Skeleton>::Container skeletons;
		Types<SkelAnim>::Container skelAnims;
		Types<LightRsrc>::Container lightProps;
		Types<ParticleEmitterRsrc>::Container particleEmitterProps;
		Types<Script>::Container scripts;
		Types<Model>::Container models;
		Types<Skin>::Container skins;
		Types<DummyRsrc>::Container dummies;
		/// @}

		RsrcAsyncLoadingReqsHandler rsrcAsyncLoadingReqsHandler;
		
		/// This will be used in every new texture until the async loader is
		/// finished with the loading of the actual texture. Its initialized
		/// when its first needed so that we wont have conflicts with opengl
		/// initialization.
		boost::scoped_ptr<Texture> dummyTex;

		/// The same as dummyTex but for normals
		boost::scoped_ptr<Texture> dummyNormTex;

		/// Find a resource using the filename
		template<typename Type>
		typename Types<Type>::Iterator find(const char* filename,
			typename Types<Type>::Container& container);

		/// Specialized func
		template<typename Type>
		typename Types<Type>::Container& choseContainer();

		/// Unload a resource if no one uses it. This is the real deal
		template<typename Type>
		void unloadR(const typename Types<Type>::Hook& info);
		
		/// Allocate and load a resource.
		/// This method allocates memory for a resource and loads it (calls the
		/// load method). Its been used by the load method. Its a separate
		/// method because we want to specialize it for async loaded resources
		template<typename Type>
		void allocAndLoadRsrc(const char* filename, Type*& ptr);

		/// Dealocate the resource. Its separate for two reasons:
		/// - Because we want to specialize it for the async loaded resources
		/// - Because we cannot have the operator delete in a template body.
		///   Apparently the compiler is to dump to decide
		template<typename Type>
		void deallocRsrc(Type* rsrc);
};


inline void ResourceManager::postProcessFinishedLoadingRequests(float maxTime)
{
	rsrcAsyncLoadingReqsHandler.postProcessFinishedRequests(maxTime);
}


inline size_t ResourceManager::getAsyncLoadingRequestsNum() const
{
	return rsrcAsyncLoadingReqsHandler.getRequestsNum();
}


#include "ResourceManager.inl.h"


#endif
