#ifndef _MESH_H_
#define _MESH_H_

#include "Common.h"
#include "Math.h"
#include "Vbo.h"
#include "Resource.h"
#include "collision.h"


/// Mesh @ref Resource
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
				uint vertIds[3]; // an array with the vertex indexes in the mesh class
				Vec3 normal;
		};

		Vec<Vec3>         vertCoords;
		Vec<Vec3>         vertNormals;
		Vec<Vec4>         vertTangents;
		Vec<Vec2>         texCoords;    ///< One for every vert so we can use vertex arrays & VBOs
		Vec<VertexWeight> vertWeights;
		Vec<Triangle>     tris;
		Vec<ushort>       vertIndeces; ///< Used for vertex arrays & VBOs

		struct
		{
			Vbo vertCoords;
			Vbo vertNormals;
			Vbo vertTangents;
			Vbo texCoords;
			Vbo vertIndeces;
			Vbo vertWeights;
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
