#ifndef ANKI_RESOURCE_MESH_LOADER_H
#define ANKI_RESOURCE_MESH_LOADER_H

#include "anki/math/Math.h"
#include <boost/range/iterator_range.hpp>
#include <boost/array.hpp>
#include <string>
#include <vector>

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
		boost::array<float, MAX_BONES_PER_VERT> boneIds;
		boost::array<float, MAX_BONES_PER_VERT> weights;
	};

	/// Triangle
	class Triangle
	{
		public:
			/// An array with the vertex indexes in the mesh class
			boost::array<uint, 3> vertIds;
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
	const std::vector<Vec3>& getVertCoords() const
	{
		return vertCoords;
	}

	const std::vector<Vec3>& getVertNormals() const
	{
		return vertNormals;
	}

	const std::vector<Vec4>& getVertTangents() const
	{
		return vertTangents;
	}

	const std::vector<Vec2>& getTexCoords() const
	{
		return texCoords;
	}

	const std::vector<VertexWeight>& getVertWeights() const
	{
		return vertWeights;
	}

	const std::vector<Triangle>& getTris() const
	{
		return tris;
	}

	const std::vector<ushort>& getVertIndeces() const
	{
		return vertIndeces;
	}
	/// @}

private:
	/// @name Data
	/// @{
	std::vector<Vec3> vertCoords; ///< Loaded from file
	std::vector<Vec3> vertNormals; ///< Generated
	std::vector<Vec4> vertTangents; ///< Generated
	/// Optional. One for every vert so we can use vertex arrays & VBOs
	std::vector<Vec2> texCoords;
	std::vector<VertexWeight> vertWeights; ///< Optional
	std::vector<Triangle> tris; ///< Required
	/// Generated. Used for vertex arrays & VBOs
	std::vector<ushort> vertIndeces;
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


} // end namespace


#endif
