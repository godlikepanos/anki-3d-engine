// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURCE_MESH_LOADER_H
#define ANKI_RESOURCE_MESH_LOADER_H

#include "anki/resource/Common.h"
#include "anki/Math.h"
#include "anki/util/Vector.h"
#include "anki/util/Array.h"

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
	template<typename T>
	using MLDArray = TempResourceDArray<T>;

	/// If two vertices have the same position and normals under the angle 
	/// specified by this constant then combine those normals
	static constexpr F32 NORMALS_ANGLE_MERGE = getPi<F32>() / 6;

	/// Vertex weight for skeletal animation
	class VertexWeight
	{
	public:
		/// Dont change this or prepare to change the skinning code in
		/// shader
		static const U32 MAX_BONES_PER_VERT = 4;

		U16 m_bonesCount;
		Array<U16, MAX_BONES_PER_VERT> m_boneIds;
		Array<F16, MAX_BONES_PER_VERT> m_weights;
	};

	/// Triangle
	class Triangle
	{
	public:
		/// An array with the vertex indexes in the mesh class
		Array<U32, 3> m_vertIds;
		Vec3 m_normal;
	};

	MeshLoader(TempResourceAllocator<U8>& alloc);

	~MeshLoader();

	/// @name Accessors
	/// @{
	const MLDArray<Vec3>& getPositions() const
	{
		return m_positions;
	}

	const MLDArray<HVec3>& getNormals() const
	{
		return m_normalsF16;
	}

	const MLDArray<HVec4>& getTangents() const
	{
		return m_tangentsF16;
	}

	const MLDArray<HVec2>& getTextureCoordinates(const U32 channel) const
	{
		return m_texCoordsF16;
	}
	U getTextureChannelsCount() const
	{
		return 1;
	}

	const MLDArray<VertexWeight>& getWeights() const
	{
		return m_weights;
	}

	const MLDArray<U16>& getIndices() const
	{
		return m_vertIndices;
	}
	/// @}

	/// Append data from another mesh loader. BucketMesh method
	ANKI_USE_RESULT Error append(const MeshLoader& other);

	/// Load the mesh data from a binary file
	/// @exception Exception
	ANKI_USE_RESULT Error load(const CString& filename);

private:
	TempResourceAllocator<U8> m_alloc;

	MLDArray<Vec3> m_positions; ///< Loaded from file

	MLDArray<Vec3> m_normals; ///< Generated
	MLDArray<HVec3> m_normalsF16;

	MLDArray<Vec4> m_tangents; ///< Generated
	MLDArray<HVec4> m_tangentsF16;

	/// Optional. One for every vert so we can use vertex arrays & VBOs
	MLDArray<Vec2> m_texCoords;
	MLDArray<HVec2> m_texCoordsF16;

	MLDArray<VertexWeight> m_weights; ///< Optional

	MLDArray<Triangle> m_tris; ///< Required

	/// Generated. Used for vertex arrays & VBOs
	MLDArray<U16> m_vertIndices;

	ANKI_USE_RESULT Error createFaceNormals();
	ANKI_USE_RESULT Error createVertNormals();
	ANKI_USE_RESULT Error createAllNormals()
	{
		Error err = createFaceNormals();
		if(!err)
		{
			err = createVertNormals();
		}
		return err;
	}
	ANKI_USE_RESULT Error createVertTangents();
	ANKI_USE_RESULT Error createVertIndeces();

	/// This method does some sanity checks and creates normals,
	/// tangents, VBOs etc
	/// @exception Exception
	ANKI_USE_RESULT Error doPostLoad();

	/// It iterates all verts and fixes the normals on seams
	void fixNormals();

	/// Compress some buffers for increased BW performance
	ANKI_USE_RESULT Error compressBuffers();
};

} // end namespace anki

#endif
