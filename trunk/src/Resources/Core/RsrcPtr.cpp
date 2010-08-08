#include "RsrcPtr.h"
#include "RsrcContainer.h"
#include "Texture.h"
#include "Material.h"
#include "ShaderProg.h"
#include "Mesh.h"
#include "Skeleton.h"
#include "SkelAnim.h"
#include "LightProps.h"
#include "ParticleEmitterProps.h"


namespace RsrcContainers {

extern RsrcContainer<Texture>    textures;
extern RsrcContainer<ShaderProg> shaderProgs;
extern RsrcContainer<Material>   materials;
extern RsrcContainer<Mesh>       meshes;
extern RsrcContainer<Skeleton>   skeletons;
extern RsrcContainer<SkelAnim>   skelAnims;
extern RsrcContainer<LightProps> lightProps;
extern RsrcContainer<ParticleEmitterProps> particleEmitterProps;

}


#define LOAD_RSRC(container) \
	p = RsrcContainers::container.load(filename); \
	return p != NULL;


#define UNLOAD_RSRC(container) \
	if(p) \
	{ \
		RsrcContainers::container.unload(p); \
		p = NULL; \
	}


//======================================================================================================================
// loadRsrc <Texture>                                                                                                  =
//======================================================================================================================
template<>
bool RsrcPtr<Texture>::loadRsrc(const char* filename)
{
	LOAD_RSRC(textures);
}


//======================================================================================================================
// loadRsrc <ShaderProg>                                                                                               =
//======================================================================================================================
template<>
bool RsrcPtr<ShaderProg>::loadRsrc(const char* filename)
{
	LOAD_RSRC(shaderProgs);
}


//======================================================================================================================
// loadRsrc <Material>                                                                                                 =
//======================================================================================================================
template<>
bool RsrcPtr<Material>::loadRsrc(const char* filename)
{
	LOAD_RSRC(materials);
}


//======================================================================================================================
// loadRsrc <Mesh>                                                                                                     =
//======================================================================================================================
template<>
bool RsrcPtr<Mesh>::loadRsrc(const char* filename)
{
	LOAD_RSRC(meshes);
}


//======================================================================================================================
// loadRsrc <Skeleton>                                                                                                 =
//======================================================================================================================
template<>
bool RsrcPtr<Skeleton>::loadRsrc(const char* filename)
{
	LOAD_RSRC(skeletons);
}


//======================================================================================================================
// loadRsrc <SkelAnim>                                                                                                 =
//======================================================================================================================
template<>
bool RsrcPtr<SkelAnim>::loadRsrc(const char* filename)
{
	LOAD_RSRC(skelAnims);
}


//======================================================================================================================
// loadRsrc <LightProp>                                                                                                =
//======================================================================================================================
template<>
bool RsrcPtr<LightProps>::loadRsrc(const char* filename)
{
	LOAD_RSRC(lightProps);
}


//======================================================================================================================
// loadRsrc <ParticleEmitterProps>                                                                                     =
//======================================================================================================================
template<>
bool RsrcPtr<ParticleEmitterProps>::loadRsrc(const char* filename)
{
	LOAD_RSRC(particleEmitterProps);
}


//======================================================================================================================
// unload <Texture>                                                                                                    =
//======================================================================================================================
template<>
void RsrcPtr<Texture>::unload()
{
	UNLOAD_RSRC(textures);
}


//======================================================================================================================
// unload <ShaderProg>                                                                                                    =
//======================================================================================================================
template<>
void RsrcPtr<ShaderProg>::unload()
{
	UNLOAD_RSRC(shaderProgs);
}


//======================================================================================================================
// unload <Material>                                                                                                    =
//======================================================================================================================
template<>
void RsrcPtr<Material>::unload()
{
	UNLOAD_RSRC(materials);
}


//======================================================================================================================
// unload <Mesh>                                                                                                       =
//======================================================================================================================
template<>
void RsrcPtr<Mesh>::unload()
{
	UNLOAD_RSRC(meshes);
}


//======================================================================================================================
// unload <Skeleton>                                                                                                   =
//======================================================================================================================
template<>
void RsrcPtr<Skeleton>::unload()
{
	UNLOAD_RSRC(skeletons);
}



//======================================================================================================================
// unload <SkelAnim>                                                                                                   =
//======================================================================================================================
template<>
void RsrcPtr<SkelAnim>::unload()
{
	UNLOAD_RSRC(skelAnims);
}


//======================================================================================================================
// unload <LightProps>                                                                                                 =
//======================================================================================================================
template<>
void RsrcPtr<LightProps>::unload()
{
	UNLOAD_RSRC(lightProps);
}


//======================================================================================================================
// unload <ParticleEmitterProps>                                                                                       =
//======================================================================================================================
template<>
void RsrcPtr<ParticleEmitterProps>::unload()
{
	UNLOAD_RSRC(particleEmitterProps);
}
