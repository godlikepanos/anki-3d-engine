#ifndef ANKI_RESOURCE_MESH_H
#define ANKI_RESOURCE_MESH_H

#include "anki/Math.h"
#include "anki/gl/GlBuffer.h"
#include "anki/collision/Obb.h"
#include "anki/util/Vector.h"

namespace anki {

class MeshLoader;

/// Mesh Resource. It contains the geometry packed in VBOs
class Mesh
{
public:
	enum VertexAttribute
	{
		VA_POSITION,
		VA_NORMAL,
		VA_TANGENT,
		VA_TEXTURE_COORD,
		VA_TEXTURE_COORD_1,
		VA_BONE_COUNT,
		VA_BONE_IDS,
		VA_BONE_WEIGHTS,
		VA_INDICES, 
		VA_COUNT
	};

	/// Default constructor. Do nothing
	Mesh()
	{}

	/// Does nothing
	~Mesh()
	{}

	U32 getTextureChannelsCount() const
	{
		return texChannelsCount;
	}

	Bool hasWeights() const
	{
		return weights;
	}

	/// Used only to clone the VBO
	U32 getVerticesCount() const
	{
		return vertsCount;
	}

	U32 getIndicesCount() const
	{
		return indicesCount;
	}

	const Obb& getBoundingShape() const
	{
		return obb;
	}

	/// Get indices count and offset of submesh
	U32 getIndicesCountSub(U subMeshId, U32& offset) const
	{
		ANKI_ASSERT(subMeshId < subMeshes.size());
		const SubMesh& sm = subMeshes[subMeshId];
		offset = sm.indicesOffset;
		return sm.indicesCount;
	}

	const Obb& getBoundingShapeSub(U subMeshId) const
	{
		ANKI_ASSERT(subMeshId < subMeshes.size());
		return subMeshes[subMeshId].obb;
	}

	/// If returns zero then the mesh is a single uniform mesh
	U32 getSubMeshesCount() const
	{
		return subMeshes.size();
	}

	/// Get info on how to attach a VBO to a VAO
	void getVboInfo(
		const VertexAttribute attrib, const GlBuffer*& vbo,
		U32& size, GLenum& type, U32& stride, U32& offset) const;

	/// Helper function for correct loading
	Bool isCompatible(const Mesh& other) const;

	/// Load from a .mesh file
	void load(const char* filename);

protected:
	/// Per sub mesh data
	struct SubMesh
	{
		U32 indicesCount;
		U32 indicesOffset;
		Obb obb;
	};

	Vector<SubMesh> subMeshes;
	U32 indicesCount;
	U32 vertsCount;
	Obb obb;
	U8 texChannelsCount;
	Bool8 weights;

	GlBuffer vbo;
	GlBuffer indicesVbo;

	/// Create the VBOs using the mesh data
	void createVbos(const MeshLoader& loader);

	U32 calcVertexSize() const;
};

/// A mesh that behaves as a mesh and as a collection of separate meshes
class BucketMesh: public Mesh
{
public:
	/// Default constructor. Do nothing
	BucketMesh()
	{}

	/// Load file
	BucketMesh(const char* filename)
	{
		load(filename);
	}
	/// @}

	/// Does nothing
	~BucketMesh()
	{}

	/// Load from a .mmesh file
	void load(const char* filename);
};

} // end namespace anki

#endif
