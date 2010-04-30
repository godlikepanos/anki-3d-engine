#include "Resource.h"
#include "Texture.h"
#include "Material.h"
#include "ShaderProg.h"
#include "Mesh.h"
#include "SkelAnim.h"
#include "LightProps.h"


namespace Rsrc {


/*
=======================================================================================================================================
DATA OBJECTS                                                                                                                          =
=======================================================================================================================================
*/
Container<Texture>    textures;
Container<ShaderProg> shaders;
Container<Material>   materials;
Container<Mesh>       meshes;
Container<Skeleton>   skeletons;
Container<SkelAnim>   skelAnims;
Container<LightProps> lightProps;

} // end namespace
