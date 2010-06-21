#include "Resource.h"
#include "Texture.h"
#include "Material.h"
#include "ShaderProg.h"
#include "Mesh.h"
#include "SkelAnim.h"
#include "LightProps.h"


//======================================================================================================================
// Statics                                                                                                             =
//======================================================================================================================
ResourceContainer<Texture>    Resource::textures;
ResourceContainer<ShaderProg> Resource::shaders;
ResourceContainer<Material>   Resource::materials;
ResourceContainer<Mesh>       Resource::meshes;
ResourceContainer<Skeleton>   Resource::skeletons;
ResourceContainer<SkelAnim>   Resource::skelAnims;
ResourceContainer<LightProps> Resource::lightProps;

