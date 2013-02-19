#ifndef ANKI_RESOURCE_MESH_H
#define ANKI_RESOURCE_MESH_H

#include "anki/math/Math.h"
#include "anki/gl/Vbo.h"
#include "anki/collision/Obb.h"
#include "anki/util/Vector.h"

namespace anki {

class MeshLoader;

/// This is the interface class for meshes. Its interface because the skin
/// nodes override it
class MeshBase
{
public:
	enum VertexAttribute
	{
		VA_POSITION,
		VA_NORMAL,
		VA_TANGENT,
		VA_TEXTURE_COORDS,
		VA_TEXTURE_COORDS_1,
		VA_BONE_COUNT,
		VA_BONE_IDS,
		VA_BONE_WEIGHTS,
		VA_INDICES, 
		VA_COUNT
	};

	virtual ~MeshBase()
	{}

	/// Get info on how to attach a VBO to a VAO
	virtual void getVboInfo(
		const VertexAttribute attrib, const Vbo*& vbo,
		U32& size, GLenum& type, U32& stride, U32& offset) const = 0;

	U32 getTextureChannelsCount() const
	{
		return meshProtected.texChannelsCount;
	}

	Bool hasWeights() const
	{
		return meshProtected.weights;
	}

	/// Used only to clone the VBO
	U32 getVerticesCount() const
	{
		return meshProtected.vertsCount;
	}

	U32 getIndicesCount() const
	{
		ANKI_ASSERT(meshProtected.subMeshes.size() > 0);
		return meshProtected.subMeshes[0].indicesCount;
	}

	const Obb& getBoundingShape() const
	{
		return meshProtected.obb;
	}

	U32 getIndicesCountSub(U subMeshId, U32& offset) const
	{
		ANKI_ASSERT(subMeshId < meshProtected.subMeshes.size());
		const SubMesh& sm = meshProtected.subMeshes[subMeshId];
		offset = sm.indicesOffset;
		return sm.indicesCount;
	}

	const Obb& getBoundingShapeSub(U subMeshId) const
	{
		ANKI_ASSERT(subMeshId < meshProtected.subMeshes.size());
		return meshProtected.subMeshes[subMeshId].obb;
	}

	U32 getSubMeshesCount() const
	{
		return meshProtected.subMeshes.size();
	}

	/// Helper function for correct loading
	Bool isCompatible(const MeshBase& other) const;

protected:
	/// Per sub mesh data
	struct SubMesh
	{
		U32 indicesCount;
		U32 indicesOffset;
		Obb obb;
	};

	struct
	{
		Vector<SubMesh> subMeshes;
		U32 indicesCount;
		U32 vertsCount;
		Obb obb;
		U8 texChannelsCount;
		Bool8 weights;
	} meshProtected;
};

/// Mesh Resource. It contains the geometry packed in VBOs
class Mesh: public MeshBase
{
public:
	/// @name Constructors
	/// @{

	/// Default constructor. Do nothing
	Mesh()
	{}

	/// Load file
	Mesh(const char* filename)
	{
		load(filename);
	}
	/// @}

	/// Does nothing
	~Mesh()
	{}

	/// @name MeshBase implementers
	/// @{
	void getVboInfo(
		const VertexAttribute attrib, const Vbo*& vbo,
		U32& size, GLenum& type, U32& stride, U32& offset) const;
	/// @}

	/// Load from a .mesh file
	void load(const char* filename);

protected:
	Vbo vbo;
	Vbo indicesVbo;

	/// Create the VBOs using the mesh data
	void createVbos(const MeshLoader& loader);

	U32 calcVertexSize() const;
};

/// A mesh that behaves as a mesh and as a collection of separate meshes
class BucketMesh: public Mesh
{
public:
	/// The absolute limit of submeshes
	static const U MAX_SUB_MESHES = 64;

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
