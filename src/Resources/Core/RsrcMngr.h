#ifndef RSRCMNGR_H
#define RSRCMNGR_H

#include "Common.h"
#include "RsrcContainer.h"

class Texture;
class Material;
class ShaderProg;
class Mesh;
class Skeleton;
class SkelAnim;
class LightProps;


namespace RsrcMngr {

extern RsrcContainer<Texture>    textures;
extern RsrcContainer<ShaderProg> shaders;
extern RsrcContainer<Material>   materials;
extern RsrcContainer<Mesh>       meshes;
extern RsrcContainer<Skeleton>   skeletons;
extern RsrcContainer<SkelAnim>   skelAnims;
extern RsrcContainer<LightProps> lightProps;

}


#endif
