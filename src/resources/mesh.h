#ifndef _MESH_H_
#define _MESH_H_

#include "common.h"
#include "gmath.h"
#include "vbo.h"
#include "resource.h"
#include "collision.h"


/// Mesh resource
class mesh_t: public resource_t
{
	// data
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

		class triangle_t
		{
			public:
				uint   vert_ids[3]; // an array with the vertex indexes in the mesh class
				vec3_t normal;
		};

		vec_t<vec3_t>          vert_coords;
		vec_t<vec3_t>          vert_normals;
		vec_t<vec4_t>          vert_tangents;
		vec_t<vec2_t>          tex_coords;    ///< One for every vert so we can use vertex arrays
		vec_t<vertex_weight_t> vert_weights;
		vec_t<triangle_t>      tris;
		vec_t<ushort>          vert_indeces; ///< Used for vertex arrays

		struct
		{
			vbo_t vert_coords;
			vbo_t vert_normals;
			vbo_t vert_tangents;
			vbo_t tex_coords;
			vbo_t vert_indeces;
			vbo_t vert_weights;
		} vbos;

		string material_name;

		bsphere_t bsphere;

	// funcs
	protected:
		void CreateFaceNormals();
		void CreateVertNormals();
		void CreateAllNormals() { CreateFaceNormals(); CreateVertNormals(); }
		void CreateVertTangents();
		void CreateVertIndeces();
		void CreateVBOs();
		void CalcBSphere();

	public:
		mesh_t() {}
		virtual ~mesh_t() { /*ToDo*/ }
		bool Load( const char* filename );
		void Unload();

};


#endif
