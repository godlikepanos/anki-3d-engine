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

	virtual U32 getTextureChannelsCount() const = 0;

	virtual Bool hasWeights() const = 0;

	/// Used only to clone the VBO
	virtual U32 getVerticesCount() const = 0;

	virtual U32 getIndicesCount() const = 0;

	virtual const Obb& getBoundingShape() const = 0;

	virtual U32 getIndicesCountSub(U32 subMeshId, U32& offset) const
	{
		ANKI_ASSERT(subMeshId == 0);
		offset = 0;
		return getIndicesCount();
	}

	virtual const Obb& getBoundingShapeSub(U32 subMeshId) const
	{
		ANKI_ASSERT(subMeshId == 0);
		return getBoundingShape();
	}

	virtual U32 getSubMeshesCount() const
	{
		return 1;
	}
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
	U32 getVerticesCount() const
	{
		return vertsCount;
	}

	U32 getIndicesCount() const
	{
		return indicesCount;
	}

	U32 getTextureChannelsCount() const
	{
		return texChannelsCount;
	}

	Bool hasWeights() const
	{
		return weights;
	}

	const Obb& getBoundingShape() const
	{
		return visibilityShape;
	}

	void getVboInfo(
		const VertexAttribute attrib, const Vbo*& vbo,
		U32& size, GLenum& type, U32& stride, U32& offset) const;
	/// @}

	/// Load from a .mesh file
	void load(const char* filename);

protected:
	U32 vertsCount;
	U32 indicesCount; ///< Indices count per level
	U32 texChannelsCount;
	Bool8 weights;
	Obb visibilityShape;

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

	/// @name MeshBase implementers
	/// @{
	U32 getIndicesCountSub(U32 subMeshId, U32& offset) const
	{
		ANKI_ASSERT(subMeshId < subMeshes.size());
		offset = subMeshes[subMeshId].indicesOffset;
		return subMeshes[subMeshId].indicesCount;
	}

	const Obb& getBoundingShapeSub(U32 subMeshId) const
	{
		ANKI_ASSERT(subMeshId < subMeshes.size());
		return subMeshes[subMeshId].visibilityShape;
	}

	U32 getSubMeshesCount() const
	{
		return subMeshes.size();
	}
	/// @}

	/// Load from a .mmesh file
	void load(const char* filename);

private:
	struct SubMeshData
	{
		U32 indicesCount;
		U32 indicesOffset; ///< In bytes
		Obb visibilityShape;
	};

	Vector<SubMeshData> subMeshes;
};

} // end namespace anki

#endif
