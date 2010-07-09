#include "RsrcMngr.h"
#include "RsrcContainer.h"
#include "Texture.h"
#include "Material.h"
#include "ShaderProg.h"
#include "Mesh.h"
#include "Skeleton.h"
#include "SkelAnim.h"
#include "LightProps.h"


namespace RsrcMngr {

RsrcContainer<Texture>    textures;
RsrcContainer<ShaderProg> shaders;
RsrcContainer<Material>   materials;
RsrcContainer<Mesh>       meshes;
RsrcContainer<Skeleton>   skeletons;
RsrcContainer<SkelAnim>   skelAnims;
RsrcContainer<LightProps> lightProps;

}
