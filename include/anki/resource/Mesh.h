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
		const VertexAttribute attrib, const U32 lod, const Vbo*& vbo, 
		U32& size, GLenum& type, U32& stride, U32& offset) const = 0;

	U32 getVerticesCount() const
	{
		return vertsCount;
	}

	U32 getIndicesCount(U32 lod) const
	{
		return indicesCount[lod];
	}

	U32 getTextureChannelsCount() const
	{
		return texChannelsCount;
	}

	Bool hasWeights() const
	{
		return weights;
	}

	U32 getLodsCount() const
	{
		return indicesCount.size();
	}

	const Obb& getBoundingShape() const
	{
		return visibilityShape;
	}

protected:
	U32 vertsCount;
	Vector<U32> indicesCount; ///< Indices count per level
	U32 texChannelsCount;
	Bool weights;
	Obb visibilityShape;
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

	/// Load from a file
	void load(const char* filename);

	/// Implements MeshBase::getVboInfo
	void getVboInfo(
		const VertexAttribute attrib, const U32 lod, const Vbo*& vbo, 
		U32& size, GLenum& type, U32& stride, U32& offset) const;

private:
	Vbo vbo;
	Vector<Vbo> indicesVbos;

	/// Create the VBOs using the mesh data
	void createVbos(const MeshLoader& meshData);

	U32 calcVertexSize() const;
};

} // end namespace anki

#endif
