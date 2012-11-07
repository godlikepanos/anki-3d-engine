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
		VA_POSITIONS,
		VA_NORMALS,
		VA_TANGENTS,
		VA_TEXTURE_COORDS,
		VA_WEIGHTS_BONE_COUNT,
		VA_WEIGHTS_BONE_IDS,
		VA_WEIGHTS_BONE_WEIGHTS,
		VA_INDICES, 
		VA_COUNT
	};

	virtual ~MeshBase()
	{}

	/// Get info on how to attach a VBO to a VAO
	virtual void getVboInfo(
		const VertexAttribute attrib, const U32 lod, const U32 texChannel,
		Vbo* vbo, U32& size, GLenum& type, U32& stride, U32& offset) = 0;

	virtual U32 getVerticesCount() const = 0;

	virtual U32 getIndicesCount(U32 lod) const = 0;

	virtual U32 getTextureChannelsCount() const = 0;

	virtual Bool hasWeights() const = 0;

	virtual U32 getLodsCount() const = 0;

	virtual const Obb& getBoundingShape() const = 0;
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

	/// @name Accessors
	/// @{

	/// Implements MeshBase::getVerticesCount
	U32 getVerticesCount() const
	{
		return vertsCount;
	}

	/// Implements MeshBase::getIndicesCount
	U32 getIndicesCount(U32 lod) const
	{
		return indicesCount[lod];
	}

	/// Implements MeshBase::getTextureChannelsCount
	U32 getTextureChannelsCount() const
	{
		return texChannelsCount;
	}

	/// Implements MeshBase::hasWeights
	Bool hasWeights() const
	{
		return weights;
	}

	/// Implements MeshBase::getLodsCount
	U32 getLodsCount() const
	{
		return indicesVbos.size();
	}

	/// Implements MeshBase::getBoundingShape
	const Obb& getBoundingShape() const
	{
		return visibilityShape;
	}
	/// @}

	/// Load from a file
	void load(const char* filename);

	/// Implements MeshBase::getVboInfo
	void getVboInfo(
		const VertexAttribute attrib, const U32 lod, const U32 texChannel,
		Vbo* vbo, U32& size, GLenum& type, U32& stride, U32& offset);

private:
	Vbo vbo;
	Vector<Vbo> indicesVbos;

	U32 vertsCount;
	Vector<U32> indicesCount; ///< Indices count per level
	Bool weights;
	U32 texChannelsCount;

	Obb visibilityShape;

	/// Create the VBOs using the mesh data
	void createVbos(const MeshLoader& meshData);

	U32 calcVertexSize() const;
};

} // end namespace anki

#endif
