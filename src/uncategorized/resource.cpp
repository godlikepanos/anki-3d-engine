#include "resource.h"
#include "texture.h"
#include "material.h"
#include "shader_prog.h"
#include "mesh.h"
#include "smodel.h"
#include "model.h"


namespace rsrc {


/*
=======================================================================================================================================
DATA OBJECTS                                                                                                                          =
=======================================================================================================================================
*/
container_t<texture_t>     textures;
container_t<shader_prog_t> shaders;
container_t<material_t>    materials;
container_t<mesh_data_t>   mesh_datas;
container_t<model_data_t>  model_datas;
container_t<model_t>       models;


} // end namespace
