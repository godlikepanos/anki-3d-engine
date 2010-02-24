#ifndef _MESH_H_
#define _MESH_H_

#include "common.h"
#include "gmath.h"
#include "vbo.h"
#include "Resource.h"
#include "collision.h"


/// Mesh resource
class Mesh: public Resource
{
	// data
	public:
		class VertexWeight
		{
			public:
				static const uint MAX_BONES_PER_VERT = 4; ///< Dont change or change the skinning code in shader

				// ToDo: change the vals to uint when change drivers
				float bonesNum;
				float boneIds[MAX_BONES_PER_VERT];
				float weights[MAX_BONES_PER_VERT];
		};

		class Triangle
		{
			public:
				uint   vertIds[3]; // an array with the vertex indexes in the mesh class
				vec3_t normal;
		};

		Vec<vec3_t>       vertCoords;
		Vec<vec3_t>       vertNormals;
		Vec<vec4_t>       vertTangents;
		Vec<vec2_t>       texCoords;    ///< One for every vert so we can use vertex arrays
		Vec<VertexWeight> vertWeights;
		Vec<Triangle>     tris;
		Vec<ushort>       vertIndeces; ///< Used for vertex arrays

		struct
		{
			vbo_t vertCoords;
			vbo_t vertNormals;
			vbo_t vertTangents;
			vbo_t texCoords;
			vbo_t vertIndeces;
			vbo_t vertWeights;
		} vbos;

		string materialName;

		bsphere_t bsphere;

	// funcs
	protected:
		void createFaceNormals();
		void createVertNormals();
		void createAllNormals() { createFaceNormals(); createVertNormals(); }
		void createVertTangents();
		void createVertIndeces();
		void createVBOs();
		void calcBSphere();

	public:
		Mesh() {}
		virtual ~Mesh() { /*ToDo*/ }
		bool load( const char* filename );
		void unload();

};


#endif
