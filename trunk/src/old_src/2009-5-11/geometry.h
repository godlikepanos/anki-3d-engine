#ifndef _GEOMETRY_H_
#define _GEOMETRY_H_

#include <iostream>
#include <fstream>
#include "common.h"
#include "shaders.h"
#include "math.h"
#include "primitives.h"
#include "engine_class.h"

class mesh_data_t: public data_class_t
{
	public:
		vector<vertex_t>   verts;
		vector<triangle_t> tris;
		vector<vec2_t>     uvs; // one for every vert so we can use vert arrays

		vector<vec3_t>     face_tangents;
		vector<vec3_t>     vert_tangents;
		vector<vec3_t>     face_bitangents;
		vector<vec3_t>     vert_bitangents;

		vector<uint>       vert_list; // for vertex arrays

		mesh_data_t() {}
		~mesh_data_t() {}

		bool Load( const char* filename );
		void Unload() { verts.clear(); tris.clear(); uvs.clear(); face_tangents.clear(); vert_tangents.clear(); vert_list.clear(); }
		void CalcFaceNormals();
		void CalcVertNormals();
		void CalcAllNormals() { CalcFaceNormals(); CalcVertNormals(); }
		void CalcFaceTangents();
		void CalcVertTangents();
		void CalcAllTangents() { CalcFaceTangents(); CalcVertTangents(); }
};


// mesh
class mesh_runtime_class_t: public runtime_class_t {}; // for ambiguity reasons

class mesh_t: public object_t, public mesh_runtime_class_t
{
	protected:
		mesh_data_t* mesh_data;

	public:
		void Init( mesh_data_t* meshd ) { mesh_data = meshd; }
		void Render();
};


#endif
