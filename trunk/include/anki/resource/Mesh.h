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
	Bool weights;
	Obb visibilityShape;

	Vbo vbo;
	Vbo indicesVbo;

	/// Create the VBOs using the mesh data
	void createVbos(const MeshLoader& loader);

	U32 calcVertexSize() const;
};

/// A mesh that behaves as a mesh and as a collection of separate meshes
class MultiMesh: public Mesh
{
	/// Default constructor. Do nothing
	MultiMesh()
	{}

	/// Load file
	MultiMesh(const char* filename)
	{
		load(filename);
	}
	/// @}

	/// Does nothing
	~MultiMesh()
	{}

	/// @name MeshBase implementers
	/// @{
	U32 getIndicesCountSub(U32 subMeshId, U32& offset) const
	{
		ANKI_ASSERT(subMeshId < subIndicesCount.size());
		offset = subIndicesOffsets[subMeshId];
		return subIndicesCount[subMeshId];
	}

	const Obb& getBoundingShapeSub(U32 subMeshId) const
	{
		ANKI_ASSERT(subMeshId < subVisibilityShapes.size());
		return subVisibilityShapes[subMeshId];
	}

	U32 getSubMeshesCount() const
	{
		return subIndicesCount.size();
	}
	/// @}

	/// Load from a .mmesh file
	void load(const char* filename);

private:
	Vector<U32> subIndicesCount;
	Vector<U32> subIndicesOffsets;
	Vector<Obb> subVisibilityShapes;
};

} // end namespace anki

#endif
