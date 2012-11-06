#ifndef ANKI_RESOURCE_MESH_H
#define ANKI_RESOURCE_MESH_H

#include "anki/math/Math.h"
#include "anki/gl/Vbo.h"
#include "anki/collision/Obb.h"

namespace anki {

class MeshLoader;

/// This is the interface class for meshes. Its interface because the skin
/// nodes override it
class MeshBase
{
public:
	/// XXX Rename to VertexAttributes
	enum VboId
	{
		VBO_POSITIONS, ///< VBO never empty
		VBO_NORMALS, ///< VBO never empty
		VBO_TANGENTS, ///< VBO never empty
		VBO_TEX_COORDS, ///< VBO may be empty
		VBO_INDICES, ///< VBO never empty
		VBO_WEIGHTS, ///< VBO may be empty
		VBOS_NUMBER
	};

	enum VertexAttribute
	{
		VA_POSITIONS,
		VA_NORMALS,
		VA_TANGENTS,
		VA_TEX_COORDS,
		VA_INDICES, 
		VA_WEIGHTS, 
		VA_COUNT
	};

	virtual ~MeshBase()
	{}

	/// @name Accessors
	/// @{

	/// Get a VBO. Its nullptr if it does not exist
	virtual const Vbo* getVbo(VboId id) const = 0;

	virtual U32 getTextureChannelsNumber() const = 0;
	virtual U32 getLodsNumber() const = 0;
	virtual U32 getIndicesNumber(U32 lod) const = 0;
	virtual U32 getVerticesNumber(U32 lod) const = 0;

	virtual const Obb& getBoundingShape() const = 0;
	/// @}

	/// @name Ask for geometry properties
	/// @{
	bool hasTexCoords() const
	{
		return getTextureChannelsNumber() > 0;
	}

	bool hasWeights() const
	{
		return getVbo(VBO_WEIGHTS) != nullptr;
	}

	bool hasNormalsAndTangents() const
	{
		return getVbo(VBO_NORMALS) && getVbo(VBO_TANGENTS);
	}
	/// @}

	/// XXX NEW interface
	virtual void getVboInfo(
		const VertexAttribute attrib, const U32 lod, const U32 texChannel,
		Vbo* vbo, U32& size, GLenum& type, U32& stride, U32& offset);

	virtual U32 getVerticesCount(U32 lod) const = 0;

	virtual U32 getLodsCount() const = 0;

	virtual U32 getTextureChannelsCount() const = 0;

	virtual Bool hasWeights() const = 0;
};

/// Mesh Resource. It contains the geometry packed in VBOs
class Mesh: public MeshBase
{
public:
	typedef Array<Vbo, VBOS_NUMBER> VbosArray;

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

	/// Implements MeshBase::getVbo
	const Vbo* getVbo(VboId id) const
	{
		return vbos[id].isCreated() ? &vbos[id] : NULL;
	}

	/// Implements MeshBase::getTextureChannelsNumber
	U32 getTextureChannelsNumber() const
	{
		return vbos[VBO_TEX_COORDS].isCreated() ? 1 : 0;
	}

	/// Implements MeshBase::getLodsNumber
	U32 getLodsNumber() const
	{
		return 1;
	}

	/// Implements MeshBase::getIndicesNumber
	U32 getIndicesNumber(U32) const
	{
		return vertIdsNum;
	}

	/// Implements MeshBase::getVerticesNumber
	U32 getVerticesNumber(U32) const
	{
		return vertsNum;
	}

	/// Implements MeshBase::getBoundingShape
	const Obb& getBoundingShape() const
	{
		return visibilityShape;
	}
	/// @}

	/// Load from a file
	void load(const char* filename);

private:
	VbosArray vbos; ///< The vertex buffer objects
	U32 vertIdsNum; ///< The number of vertex IDs
	Obb visibilityShape;
	U32 vertsNum;

	/// Create the VBOs using the mesh data
	void createVbos(const MeshLoader& meshData);
};

} // end namespace anki

#endif
