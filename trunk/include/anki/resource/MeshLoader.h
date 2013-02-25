#ifndef ANKI_RESOURCE_MESH_LOADER_H
#define ANKI_RESOURCE_MESH_LOADER_H

#include "anki/math/Math.h"
#include "anki/util/Vector.h"
#include "anki/util/Array.h"
#include <string>

namespace anki {

/// Mesh data. This class loads the mesh file and the Mesh class loads it to
/// the GPU
///
/// Binary file format:
///
/// @code
/// // Header
/// <magic:ANKIMESH>
/// <string:meshName>
///
/// // Verts
/// U32: verts number
/// F32: vert 0 x, F32 vert 0 y, F32: vert 0 z
/// ...
///
/// // Faces
/// U32: faces number
/// U32: tri 0 vert ID 0, U32: tri 0 vert ID 1, U32: tri 0 vert ID 2
/// ...
///
/// // Tex coords
/// U32: tex coords number
/// F32: tex coord for vert 0 x, F32: tex coord for vert 0 y
/// ...
///
/// // Bone weights
/// U32: bone weights number (equal to verts number)
/// U32: bones number for vert 0, U32: bone id for vert 0 and weight 0,
///       F32: weight for vert 0 and weight 0, ...
/// ...
/// @endcode
class MeshLoader
{
public:
	/// If two vertices have the same position and normals under the angle 
	/// specified by this constant then combine those normals
	static constexpr F32 NORMALS_ANGLE_MERGE = getPi<F32>() / 6;

	/// Vertex weight for skeletal animation
	struct VertexWeight
	{
		/// Dont change this or prepare to change the skinning code in
		/// shader
		static const U32 MAX_BONES_PER_VERT = 4;

		/// @todo change the vals to U32 when change drivers
		F32 bonesNum;
		Array<F32, MAX_BONES_PER_VERT> boneIds;
		Array<F32, MAX_BONES_PER_VERT> weights;
	};

	/// Triangle
	struct Triangle
	{
		/// An array with the vertex indexes in the mesh class
		Array<U32, 3> vertIds;
		Vec3 normal;
	};

	MeshLoader()
	{}
	MeshLoader(const char* filename)
	{
		load(filename);
	}
	~MeshLoader()
	{}

	/// @name Accessors
	/// @{
	const Vector<Vec3>& getPositions() const
	{
		return vertCoords;
	}

	const Vector<Vec3>& getNormals() const
	{
		return vertNormals;
	}

	const Vector<Vec4>& getTangents() const
	{
		return vertTangents;
	}

	const Vector<Vec2>& getTextureCoordinates(const U32 channel) const
	{
		return texCoords;
	}
	U getTextureChannelsCount() const
	{
		return 1;
	}

	const Vector<VertexWeight>& getWeights() const
	{
		return vertWeights;
	}

	const Vector<ushort>& getIndices() const
	{
		return vertIndices;
	}
	/// @}

	/// @name BucketMesh methods
	/// @{
	void appendPositions(const Vector<Vec3>& positions)
	{
		vertCoords.insert(
			vertCoords.end(), positions.begin(), positions.end());
	}

	void appendNormals(const Vector<Vec3>& normals)
	{
		vertNormals.insert(
			vertNormals.end(), normals.begin(), normals.end());
	}

	void appendTangents(const Vector<Vec4>& tangents)
	{
		vertTangents.insert(
			vertTangents.end(), tangents.begin(), tangents.end());
	}

	void appendTextureCoordinates(const Vector<Vec2>& coords, U32 channel)
	{
		ANKI_ASSERT(channel == 0 && "Currently only one channel is supported");
		texCoords.insert(texCoords.end(), coords.begin(), coords.end());
	}

	void appendWeights(const Vector<VertexWeight>& weights)
	{
		vertWeights.insert(vertWeights.end(), weights.begin(), weights.end());
	}

	/// This will adjust the indices bias
	void appendIndices(const Vector<U16>& indices, U16 bias)
	{
		for(U16 index : indices)
		{
			vertIndices.push_back(bias + index);
		}
	}
	/// @}

	/// Load the mesh data from a binary file
	/// @exception Exception
	void load(const char* filename);

private:
	/// @name Data
	/// @{
	Vector<Vec3> vertCoords; ///< Loaded from file
	Vector<Vec3> vertNormals; ///< Generated
	Vector<Vec4> vertTangents; ///< Generated
	/// Optional. One for every vert so we can use vertex arrays & VBOs
	Vector<Vec2> texCoords;
	Vector<VertexWeight> vertWeights; ///< Optional
	Vector<Triangle> tris; ///< Required
	/// Generated. Used for vertex arrays & VBOs
	Vector<U16> vertIndices;
	/// @}

	void createFaceNormals();
	void createVertNormals();
	void createAllNormals()
	{
		createFaceNormals();
		createVertNormals();
	}
	void createVertTangents();
	void createVertIndeces();

	/// This method does some sanity checks and creates normals,
	/// tangents, VBOs etc
	/// @exception Exception
	void doPostLoad();

	/// It iterates all verts and fixes the normals on seams
	void fixNormals();
};

} // end namespace anki

#endif
