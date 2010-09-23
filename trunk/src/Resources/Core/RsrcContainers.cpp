#include "RsrcContainer.h"
#include "Texture.h"
#include "Material.h"
#include "ShaderProg.h"
#include "Mesh.h"
#include "Skeleton.h"
#include "SkelAnim.h"
#include "LightData.h"
#include "ParticleEmitterProps.h"
#include "Script.h"


namespace RsrcContainers {

RsrcContainer<Texture>    textures;
RsrcContainer<ShaderProg> shaderProgs;
RsrcContainer<Material>   materials;
RsrcContainer<Mesh>       meshes;
RsrcContainer<Skeleton>   skeletons;
RsrcContainer<SkelAnim>   skelAnims;
RsrcContainer<LightData> lightProps;
RsrcContainer<ParticleEmitterProps> particleEmitterProps;
RsrcContainer<Script> scripts;

}
