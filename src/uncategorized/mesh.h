#ifndef _MESH_H_
#define _MESH_H_

#include "common.h"
#include "material.h"
#include "gmath.h"
#include "primitives.h"
#include "engine_class.h"
#include "object.h"
#include "vbo.h"

// mesh data
class mesh_data_t: public resource_t
{
	public:
		class vertex_weight_t
		{
			public:
				static const uint MAX_BONES_PER_VERT = 4; ///< Dont change or change the skinning code in shader

				// ToDo: change the vals to uint when change drivers
				float bones_num;
				float bone_ids[MAX_BONES_PER_VERT];
				float weights[MAX_BONES_PER_VERT];
		};

		vec_t<vec3_t>          vert_coords;
		vec_t<vec3_t>          vert_normals;
		vec_t<vec4_t>          vert_tangents;
		vec_t<vec2_t>          tex_coords;    ///< One for every vert so we can use vertex arrays
		vec_t<vertex_weight_t> vert_weights;

		vec_t<triangle_t> tris;
		vec_t<ushort>     vert_indeces; ///< Used for vertex arrays

		struct
		{
			vbo_t vert_coords;
			vbo_t vert_normals;
			vbo_t vert_tangents;
			vbo_t tex_coords;
			vbo_t vert_indeces;
			vbo_t vert_weights;
		} vbos;

		char* material_name;

		mesh_data_t(): material_name(NULL) {}
		virtual ~mesh_data_t() { /*ToDo*/ }

		// funcs
		bool Load( const char* filename );
		void Unload();
		void CreateFaceNormals();
		void CreateVertNormals();
		void CreateAllNormals() { CreateFaceNormals(); CreateVertNormals(); }
		void CreateVertTangents();
		void CreateVertIndeces();
		void CreateVBOs();
};


// mesh
class mesh_data_user_class_t: public data_user_class_t {}; // for ambiguity reasons

class mesh_t: public object_t, public mesh_data_user_class_t
{
	public:
		mesh_data_t* mesh_data;
		material_t* material;

		mesh_t(): object_t(MESH), mesh_data(NULL), material(NULL) {}

		bool Load( const char* filename );
		void Unload();
		void Render();
		void RenderDepth();
		void RenderNormals();
		void RenderTangents();
};


#endif
