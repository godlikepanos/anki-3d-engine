#ifndef _ASSETS_H_
#define _ASSETS_H_

#include "common.h"

#include "texture.h"
#include "material.h"
#include "shaders.h"
#include "geometry.h"
#include "model.h"


class texture_t;
class material_t;
class shader_prog_t;
class mesh_data_t;
class model_data_t;

namespace ass {

// texture
extern texture_t* SearchTxtr( const char* name );
extern texture_t* LoadTxtr( const char* filename );
extern void UnLoadTxtr( texture_t* txtr );

// material
extern material_t* SearchMat( const char* name );
extern material_t* LoadMat( const char* filename );
extern void UnLoadMat( material_t* mat );

// shaders
extern shader_prog_t* SearchShdr( const char* name );
extern shader_prog_t* LoadShdr( const char* filename );
extern void UnLoadShdr( shader_prog_t* mat );

// mesh data
extern mesh_data_t* SearchMeshD( const char* name );
extern mesh_data_t* LoadMeshD( const char* filename );
extern void UnLoadMeshD( mesh_data_t* mesh );

// model data
extern model_data_t* SearchModelD( const char* name );
extern model_data_t* LoadModelD( const char* filename );
extern void UnLoadModelD( model_data_t* mesh );

} // end namespace
#endif
