#ifndef MESH_H
#define MESH_H

#include "Common.h"
#include "Math.h"
#include "Vbo.h"
#include "Resource.h"
#include "collision.h"
#include "RsrcPtr.h"


class Material;


/// Mesh Resource. If the material name is empty then the mesh wont be rendered and no VBOs will be created
///
/// Binary file format:
///
/// @code
/// <magic:ANKIMESH>
/// <string:meshName>
/// <string:material>
/// <uint:vertsNum>
/// <float:vert[0].x> <float:vert[0].y> <float:vert[0].z> ...
/// <float:vert[vertsNum-1].x> <float:vert[vertsNum-1].y> <float:vert[vertsNum-1].z>
/// <uint:facesNum>
/// <uint:tri[0].vertIds[0]> <uint:tri[0].vertIds[1]> <uint:tri[0].vertIds[2]> ...
/// <uint:tri[facesNum-1].vertIds[0]> <uint:tri[facesNum-1].vertIds[1]> <uint:tri[facesNum-1].vertIds[2]>
/// @endcode
class Mesh: public Resource
{
	public:
		/**
		 * Vertex weight for skeletan animation
		 */
		class VertexWeight
		{
			public:
				static const uint MAX_BONES_PER_VERT = 4; ///< Dont change or change the skinning code in shader

				// ToDo: change the vals to uint when change drivers
				float bonesNum;
				float boneIds[MAX_BONES_PER_VERT];
				float weights[MAX_BONES_PER_VERT];
		};

		/**
		 * Triangle
		 */
		class Triangle
		{
			public:
				uint vertIds[3]; // an array with the vertex indexes in the mesh class
				Vec3 normal;
		};

		/**
		 * The VBOs in a structure
		 */
		struct Vbos
		{
			Vbo vertCoords;
			Vbo vertNormals;
			Vbo vertTangents;
			Vbo texCoords;
			Vbo vertIndeces;
			Vbo vertWeights;
		};

	public:
		Vec<Vec3>         vertCoords; ///< Required
		Vec<Vec3>         vertNormals; ///< Generated if renderable
		Vec<Vec4>         vertTangents; ///< Generated if renderable
		Vec<Vec2>         texCoords;    ///< Optional. One for every vert so we can use vertex arrays & VBOs
		Vec<VertexWeight> vertWeights; ///< Optional
		Vec<Triangle>     tris; ///< Required
		Vec<ushort>       vertIndeces; ///< Generated if renderable. Used for vertex arrays & VBOs
		Vbos              vbos; ///< Generated if renderable
		RsrcPtr<Material> material; ///< Required. If empty then mesh not renderable
		bsphere_t         bsphere; ///< @todo

		Mesh();
		~Mesh() {}
		void load(const char* filename);

		/// The mesh is renderable when the material is loaded
		bool isRenderable() const;

	private:
		void createFaceNormals();
		void createVertNormals();
		void createAllNormals();
		void createVertTangents();
		void createVertIndeces();
		void createVbos();
		void calcBSphere();

		/// This func does some sanity checks and creates normals, tangents, VBOs etc
		/// @exception Exception
		void doPostLoad();
};


inline Mesh::Mesh():
	Resource(RT_MESH)
{}


inline void Mesh::createAllNormals()
{
	createFaceNormals();
	createVertNormals();
}


#endif
