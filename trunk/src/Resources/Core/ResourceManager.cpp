#include "ResourceManager.h"

#include "Texture.h"
#include "Material.h"
#include "ShaderProg.h"
#include "Mesh.h"
#include "Skeleton.h"
#include "SkelAnim.h"
#include "LightData.h"
#include "ParticleEmitterProps.h"
#include "Script.h"
#include "Model.h"
#include "Skin.h"
#include "DummyRsrc.h"


// Because we are bored to write the same
#define SPECIALIZE_TEMPLATE_STUFF(type__, container__) \
	template<> \
	ResourceManager::Types<type__>::Container& ResourceManager::choseContainer<type__>() \
	{ \
		return container__; \
	} \
	\
	template<> \
	void ResourceManager::unload<type__>(const Types<type__>::Info& info) \
	{ \
		unloadR<type__>(info); \
	}


SPECIALIZE_TEMPLATE_STUFF(Texture, textures)
SPECIALIZE_TEMPLATE_STUFF(ShaderProg, shaderProgs)
SPECIALIZE_TEMPLATE_STUFF(Material, materials)
SPECIALIZE_TEMPLATE_STUFF(Mesh, meshes)
SPECIALIZE_TEMPLATE_STUFF(Skeleton, skeletons)
SPECIALIZE_TEMPLATE_STUFF(SkelAnim, skelAnims)
SPECIALIZE_TEMPLATE_STUFF(LightData, lightProps)
SPECIALIZE_TEMPLATE_STUFF(ParticleEmitterProps, particleEmitterProps)
SPECIALIZE_TEMPLATE_STUFF(Script, scripts)
SPECIALIZE_TEMPLATE_STUFF(Model, models)
SPECIALIZE_TEMPLATE_STUFF(Skin, skins)
SPECIALIZE_TEMPLATE_STUFF(DummyRsrc, dummies)
