#include "ResourceManager.h"

#include "Texture.h"
#include "Material.h"
#include "ShaderProg.h"
#include "Mesh.h"
#include "Skeleton.h"
#include "SkelAnim.h"
#include "LightRsrc.h"
#include "ParticleEmitterProps.h"
#include "Script.h"
#include "Model.h"
#include "Skin.h"
#include "DummyRsrc.h"

#include "Image.h"
#include "Logger.h"


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
ResourceManager::ResourceManager()
{}


//======================================================================================================================
// Destructor                                                                                                          =
//======================================================================================================================
ResourceManager::~ResourceManager()
{}


// Because we are bored to write the same
#define SPECIALIZE_TEMPLATE_STUFF(type__, container__) \
	template<> \
	ResourceManager::Types<type__>::Container& ResourceManager::choseContainer<type__>() \
	{ \
		return container__; \
	} \
	\
	template<> \
	void ResourceManager::deallocRsrc<type__>(type__* rsrc) \
	{ \
		delete rsrc; \
	}



//SPECIALIZE_TEMPLATE_STUFF(Texture, textures)
SPECIALIZE_TEMPLATE_STUFF(ShaderProg, shaderProgs)
SPECIALIZE_TEMPLATE_STUFF(Material, materials)
SPECIALIZE_TEMPLATE_STUFF(Mesh, meshes)
SPECIALIZE_TEMPLATE_STUFF(Skeleton, skeletons)
SPECIALIZE_TEMPLATE_STUFF(SkelAnim, skelAnims)
SPECIALIZE_TEMPLATE_STUFF(LightRsrc, lightProps)
SPECIALIZE_TEMPLATE_STUFF(ParticleEmitterProps, particleEmitterProps)
SPECIALIZE_TEMPLATE_STUFF(Script, scripts)
SPECIALIZE_TEMPLATE_STUFF(Model, models)
SPECIALIZE_TEMPLATE_STUFF(Skin, skins)
SPECIALIZE_TEMPLATE_STUFF(DummyRsrc, dummies)

//======================================================================================================================
// Texture Specializations                                                                                             =
//======================================================================================================================

template<>
ResourceManager::Types<Texture>::Container& ResourceManager::choseContainer<Texture>()
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
}

