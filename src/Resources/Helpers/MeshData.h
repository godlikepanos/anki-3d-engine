#ifndef MESH_DATA_H
#define MESH_DATA_H

#include <string>
#include "Math.h"
#include "StdTypes.h"
#include "Vec.h"
#include "Properties.h"


/// Mesh data. This class loads the mesh file and the Mesh class loads it to the GPU
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
class MeshData
{
	public:
		/// Vertex weight for skeletan animation
		class VertexWeight
		{
			public:
				static const uint MAX_BONES_PER_VERT = 4; ///< Dont change or change the skinning code in shader

				/// @todo change the vals to uint when change drivers
				float bonesNum;
				float boneIds[MAX_BONES_PER_VERT];
				float weights[MAX_BONES_PER_VERT];
		};

		/// Triangle
		class Triangle
		{
			public:
				uint vertIds[3]; ///< an array with the vertex indexes in the mesh class
				Vec3 normal;
		};


	PROPERTY_R(Vec<Vec3>, vertCoords, getVertCoords) ///< Loaded from file
	PROPERTY_R(Vec<Vec3>, vertNormals, getVertNormals) ///< Generated
	PROPERTY_R(Vec<Vec4>, vertTangents, getVertTangents) ///< Generated
	PROPERTY_R(Vec<Vec2>, texCoords, getTexCoords) ///< Optional. One for every vert so we can use vertex arrays & VBOs
	PROPERTY_R(Vec<VertexWeight>, vertWeights, getVertWeights) ///< Optional
	PROPERTY_R(Vec<Triangle>, tris, getTris) ///< Required
	PROPERTY_R(Vec<ushort>, vertIndeces, getVertIndeces) ///< Generated. Used for vertex arrays & VBOs
	PROPERTY_R(std::string, materialName, getMaterialName) ///< Required. If empty then mesh not renderable

	public:
		MeshData(const char* filename) {load(filename);}
		~MeshData() {}

	private:
		/// Load the mesh data from a binary file
		/// @exception Exception
		void load(const char* filename);

		void createFaceNormals();
		void createVertNormals();
		void createAllNormals();
		void createVertTangents();
		void createVertIndeces();

		/// This func does some sanity checks and creates normals, tangents, VBOs etc
		/// @exception Exception
		void doPostLoad();
};


inline void MeshData::createAllNormals()
{
	createFaceNormals();
	createVertNormals();
}


#endif
