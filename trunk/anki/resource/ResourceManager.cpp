#include "anki/resource/ResourceManager.h"

#include "anki/resource/Texture.h"
#include "anki/resource/Material.h"
#include "anki/resource/ShaderProgram.h"
#include "anki/resource/Mesh.h"
#include "anki/resource/Skeleton.h"
#include "anki/resource/SkelAnim.h"
#include "anki/resource/LightRsrc.h"
#include "anki/resource/ParticleEmitterRsrc.h"
#include "anki/resource/Script.h"
#include "anki/resource/Model.h"
#include "anki/resource/Skin.h"
#include "anki/resource/DummyRsrc.h"

#include "anki/resource/Image.h"
#include "anki/core/Logger.h"
#include "anki/core/Globals.h"


//==============================================================================
ResourceManager::ResourceManager()
{}


//==============================================================================
ResourceManager::~ResourceManager()
{}


//==============================================================================
template<typename Type>
void ResourceManager::allocAndLoadRsrc(const char* filename, Type*& newInstance)
{
	newInstance = NULL;

	try
	{
		newInstance = new Type();
		newInstance->load(filename);
	}
	catch(std::exception& e)
	{
		//ERROR("fuckkkkkkkkkk " << e.what());

		/*if(newInstance != NULL)
		{
			delete newInstance;
		}*/

		throw EXCEPTION("Cannot load \"" + filename + "\": " + e.what());
	}
}


//==============================================================================
template<typename Type>
typename ResourceManager::Types<Type>::Iterator ResourceManager::find(
	const char* filename, typename Types<Type>::Container& container)
{
	typename Types<Type>::Iterator it = container.begin();
	for(; it != container.end(); it++)
	{
		if(it->uuid == filename)
		{
			break;
		}
	}

	return it;
}


//==============================================================================
template<typename Type>
typename ResourceManager::Types<Type>::Hook& ResourceManager::loadR(
	const char* filename)
{
	// Chose container
	typename Types<Type>::Container& c = choseContainer<Type>();

	// Find
	typename Types<Type>::Iterator it = find<Type>(filename, c);

	// If already loaded
	if(it != c.end())
	{
		++it->referenceCounter;
	}
	// else create new, load it and update the container
	else
	{
		typename Types<Type>::Hook* hook = NULL;

		try
		{
			hook = new typename Types<Type>::Hook;
			hook->uuid = filename;
			hook->referenceCounter = 1;
			allocAndLoadRsrc<Type>(filename, hook->resource);

			c.push_back(hook);

			it = c.end();
			--it;
		}
		catch(std::exception& e)
		{
			if(hook != NULL)
			{
				delete hook;
			}

			throw EXCEPTION("Cannot load \"" + filename + "\": " + e.what());
		}
	}

	return *it;
}


//==============================================================================
template<typename Type>
void ResourceManager::unloadR(const typename Types<Type>::Hook& hook)
{
	typedef char TypeMustBeComplete[sizeof(Type) ? 1 : -1];
    (void) sizeof(TypeMustBeComplete);

	// Chose container
	typename Types<Type>::Container& c = choseContainer<Type>();

	// Find
	typename Types<Type>::Iterator it = find<Type>(hook.uuid.c_str(), c);

	// If not found
	if(it == c.end())
	{
		throw EXCEPTION("Resource hook incorrect (\"" + hook.uuid + "\")");
	}

	if(it->uuid != hook.uuid)
	{
		INFO(it->uuid << " " << hook.uuid);
	}

	ERROR(it->uuid << " " << hook.uuid << " " << it->resource << " " <<
		hook.resource << " " << dummyTex.get());
	ASSERT(it->uuid == hook.uuid);
	ASSERT(it->referenceCounter == hook.referenceCounter);

	--it->referenceCounter;

	// Delete the resource
	if(it->referenceCounter == 0)
	{
		deallocRsrc(it->resource);
		c.erase(it);
	}
}


//==============================================================================
template<typename T>
void ResourceManager::deallocRsrc(T* rsrc)
{
	delete rsrc;
}


//==============================================================================
// Template specialization                                                     =
//==============================================================================

// Because we are bored to write the same
#define SPECIALIZE_TEMPLATE_STUFF(type, container) \
	template<> \
	ResourceManager::Types<type>::Hook& \
		ResourceManager::load<type>(const char* filename) \
	{ \
		return loadR<type>(filename); \
	} \
	\
	template<> \
	void ResourceManager::unload<type>(const Types<type>::Hook& info) \
	{ \
		unloadR<type>(info); \
	} \
	\
	template<> \
	ResourceManager::Types<type>::Container& \
		ResourceManager::choseContainer<type>() \
	{ \
		return container; \
	}

SPECIALIZE_TEMPLATE_STUFF(Texture, textures)
SPECIALIZE_TEMPLATE_STUFF(ShaderProgram, shaderProgs)
SPECIALIZE_TEMPLATE_STUFF(Material, materials)
SPECIALIZE_TEMPLATE_STUFF(Mesh, meshes)
SPECIALIZE_TEMPLATE_STUFF(Skeleton, skeletons)
SPECIALIZE_TEMPLATE_STUFF(SkelAnim, skelAnims)
SPECIALIZE_TEMPLATE_STUFF(LightRsrc, lightProps)
SPECIALIZE_TEMPLATE_STUFF(ParticleEmitterRsrc, particleEmitterProps)
SPECIALIZE_TEMPLATE_STUFF(Script, scripts)
SPECIALIZE_TEMPLATE_STUFF(Model, models)
SPECIALIZE_TEMPLATE_STUFF(Skin, skins)
SPECIALIZE_TEMPLATE_STUFF(DummyRsrc, dummies)

//==============================================================================
// Texture Specializations                                                     =
//==============================================================================

/*template<>
ResourceManager::Types<Texture>::Container&
	ResourceManager::choseContainer<Texture>()
{
	return textures;
}


template<>
void ResourceManager::deallocRsrc<Texture>(Texture* rsrc)
{
	if(rsrc != dummyTex.get() && rsrc != dummyNormTex.get())
	{
		delete rsrc;
	}
}


template<>
void ResourceManager::allocAndLoadRsrc(const char* filename, Texture*& ptr)
{
	// Load the dummys
	if(dummyTex.get() == NULL)
	{
		dummyTex.reset(new Texture);
		dummyTex->load("engine-rsrc/dummy.png");
	}
	
	if(dummyNormTex.get() == NULL)
	{
		dummyNormTex.reset(new Texture);
		dummyNormTex->load("engine-rsrc/dummy.norm.png");
	}

	// Send a loading request
	rsrcAsyncLoadingReqsHandler.sendNewLoadingRequest(filename, &ptr);

	// Point to the dummy for now
	std::string fname(filename);

	size_t found = fname.find("norm.");
	if(found != std::string::npos)
	{
		ptr = dummyNormTex.get();
	}
	else
	{
		ptr = dummyTex.get();
	}
}*/

