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
/// uint: verts number
/// float: vert 0 x, float vert 0 y, float: vert 0 z
/// ...
///
/// // Faces
/// uint: faces number
/// uint: tri 0 vert ID 0, uint: tri 0 vert ID 1, uint: tri 0 vert ID 2
/// ...
///
/// // Tex coords
/// uint: tex coords number
/// float: tex coord for vert 0 x, float: tex coord for vert 0 y
/// ...
///
/// // Bone weights
/// uint: bone weights number (equal to verts number)
/// uint: bones number for vert 0, uint: bone id for vert 0 and weight 0,
///       float: weight for vert 0 and weight 0, ...
/// ...
/// @endcode
class MeshLoader
{
public:
	/// Vertex weight for skeletal animation
	class VertexWeight
	{
	public:
		/// Dont change this or prepare to change the skinning code in
		/// shader
		static const uint MAX_BONES_PER_VERT = 4;

		/// @todo change the vals to uint when change drivers
		float bonesNum;
		Array<float, MAX_BONES_PER_VERT> boneIds;
		Array<float, MAX_BONES_PER_VERT> weights;
	};

	/// Triangle
	struct Triangle
	{
		/// An array with the vertex indexes in the mesh class
		Array<uint, 3> vertIds;
		Vec3 normal;
	};

	MeshLoader(const char* filename)
	{
		load(filename);
	}
	~MeshLoader()
	{}

	/// @name Accessors
	/// @{
	const Vector<Vec3>& getVertCoords() const
	{
		return vertCoords;
	}

	const Vector<Vec3>& getVertNormals() const
	{
		return vertNormals;
	}

	const Vector<Vec4>& getVertTangents() const
	{
		return vertTangents;
	}

	const Vector<Vec2>& getTexCoords() const
	{
		return texCoords;
	}

	const Vector<VertexWeight>& getVertWeights() const
	{
		return vertWeights;
	}

	// XXX Delete: Unused
	const Vector<Triangle>& getTris() const
	{
		return tris;
	}

	const Vector<ushort>& getVertIndeces() const
	{
		return vertIndeces;
	}
	/// @}

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
	Vector<ushort> vertIndeces;
	/// @}

	/// Load the mesh data from a binary file
	/// @exception Exception
	void load(const char* filename);

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
};

} // end namespace anki

#endif
