#ifndef _GEOMETRY_H_
#define _GEOMETRY_H_

#include "primitives.h"

class mesh_data_t
{
	public:
		array_t<vertex_t>   verts;
		array_t<triangle_t> tris;
		array_t<vec2_t[3]>  uvs;
		char                name[100];

		array_t<vec4_t>     face_tangents;
		array_t<vec4_t>     vert_tangents;

		array_t<int>        vert_list;

		mesh_data_t() {}
		~mesh_data_t() { verts.Free(); tris.Free(); uvs.Free(); face_tangents.Free(); vert_tangents.Free(); vert_list.Free();};

		int  Load( const char* filename );
		void CalcFaceNormals();
		void CalcVertNormals();
		void CalcAllNormals() { CalcFaceNormals(); CalcVertNormals(); }
		void CalcFaceTangents();
		void CalcVertTangents();
		void CalcAllTangents() { CalcFaceTangents(); CalcVertTangents(); }
};


class mesh_t: public object_t
{
	public:
		mesh_data_t* mesh_data;
		void Render();
};


#endif
