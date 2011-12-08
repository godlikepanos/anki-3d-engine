#ifndef ANKI_RESOURCE_MESH_H
#define ANKI_RESOURCE_MESH_H

#include <boost/array.hpp>
#include "anki/math/Math.h"
#include "anki/resource/Resource.h"
#include "anki/gl/Vbo.h"
#include "anki/collision/Obb.h"


namespace anki {


class MeshData;


/// XXX
class MeshBase
{
public:
	MeshBase()
	{}

	virtual ~MeshBase()
	{}

	/// @name Accessors
	/// @{
	virtual const Vbo& getPositionsVbo() const = 0;
	virtual const Vbo& getNormalsVbo() const = 0;
	virtual const Vbo& getTangentsVbo() const = 0;
	virtual const Vbo& getTextureCoordsVbo(uint channel) const = 0;
	virtual const Vbo& getIndecesVbo(uint lod) const = 0;
	virtual const Vbo& getWeightsVbo() const = 0;

	virtual uint getTextureChannelsNumber() const = 0;
	virtual uint getLodsNumber() const = 0;
	virtual uint getIndicesNumber(uint lod) const = 0;
	virtual uint getVertexNumber(uint lod) const = 0;
	/// @}

	/// @name Ask for geometry properties
	/// @{
	bool hasTexCoords() const
	{
		return getTextureChannelsNumber() > 0;
	}

	virtual bool hasWeights() const;
	/// @}
};


/// Mesh Resource. It contains the geometry packed in VBOs
class Mesh
{
public:
	/// Used in @ref vbos array
	enum Vbos
	{
		VBO_VERT_POSITIONS, ///< VBO never empty
		VBO_VERT_NORMALS, ///< VBO never empty
		VBO_VERT_TANGENTS, ///< VBO never empty
		VBO_TEX_COORDS, ///< VBO may be empty
		VBO_VERT_INDECES, ///< VBO never empty
		VBO_VERT_WEIGHTS, ///< VBO may be empty
		VBOS_NUM
	};

	typedef boost::array<Vbo, VBOS_NUM> VbosArray;

	/// Default constructor
	Mesh()
	{}

	/// Does nothing
	~Mesh()
	{}

	/// @name Accessors
	/// @{
	const Vbo& getVbo(Vbos id) const
	{
		return vbos[id];
	}

	uint getVertIdsNum() const
	{
		return vertIdsNum;
	}

	const Obb& getVisibilityShape() const
	{
		return visibilityShape;
	}

	uint getVertsNum() const
	{
		return vertsNum;
	}
	/// @}

	/// Load from a file
	void load(const char* filename);

	/// @name Ask for geometry properties
	/// @{
	bool hasTexCoords() const
	{
		return vbos[VBO_TEX_COORDS].isCreated();
	}

	bool hasVertWeights() const
	{
		return vbos[VBO_VERT_WEIGHTS].isCreated();
	}

	bool hasNormalsAndTangents() const
	{
		return vbos[VBO_VERT_NORMALS].isCreated() &&
			vbos[VBO_VERT_TANGENTS].isCreated();
	}
	/// @}

private:
	VbosArray vbos; ///< The vertex buffer objects
	uint vertIdsNum; ///< The number of vertex IDs
	Obb visibilityShape;
	uint vertsNum;

	/// Create the VBOs
	void createVbos(const MeshData& meshData);
};


} // end namespace


#endif
