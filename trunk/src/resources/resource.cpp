#include "resource.h"
#include "texture.h"
#include "material.h"
#include "shader_prog.h"
#include "mesh.h"
#include "skel_anim.h"
#include "light_mtl.h"


namespace rsrc {


/*
=======================================================================================================================================
DATA OBJECTS                                                                                                                          =
=======================================================================================================================================
*/
container_t<texture_t>     textures;
container_t<shader_prog_t> shaders;
container_t<material_t>    materials;
container_t<mesh_t>        meshes;
container_t<skeleton_t>    skeletons;
container_t<skel_anim_t>   skel_anims;
container_t<light_mtl_t>   light_mtls;

} // end namespace
