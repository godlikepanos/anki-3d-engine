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
		{
			pVbo = nVbo = tVbo = wVbo = NULL;
		}

		virtual ~MeshBase()
		{}

		/// @name Accessors
		/// @{
		const Vbo& getPositionsVbo() const
		{
			ANKI_ASSERT(pVbo != NULL);
			return *pVbo;
		}

		const Vbo& getNormalsVbo() const
		{
			ANKI_ASSERT(nVbo != NULL);
			return *nVbo;
		}

		const Vbo& getTangentsVbo() const
		{
			ANKI_ASSERT(tVbo != NULL);
			return *tVbo;
		}

		const Vbo& getTextureCoordsVbo(uint channel) const
		{
			ANKI_ASSERT(texVbo[channel] != NULL);
			return *texVbo[channel];
		}

		const Vbo& getIndecesVbo(uint lod) const
		{
			ANKI_ASSERT(iVbo[lod] != NULL);
			return *iVbo[lod];
		}

		const Vbo& getWeightsVbo() const
		{
			ANKI_ASSERT(wVbo != NULL);
			return *wVbo;
		}

		uint getTextureChannelsNumber() const
		{
			return texVbo.size();
		}

		uint getLodsNumber() const
		{
			ANKI_ASSERT(iVbo.size() > 0);
			return iVbo.size();
		}

		uint getIndicesNumber(uint lod) const
		{
			ANKI_ASSERT(idsNum[lod] != 0);
			return idsNum[lod];
		}

		uint getVertexNumber(uint lod) const
		{
			ANKI_ASSERT(vertsNum[lod] != 0);
			return vertsNum[lod];
		}
		/// @}

		/// @name Ask for geometry properties
		/// @{
		bool hasTexCoords() const
		{
			return texVbo.size() > 0;
		}

		bool hasWeights() const
		{
			return wVbo != NULL;
		}
		/// @}

	protected:
		Vbo* pVbo; ///< Mandatory
		Vbo* nVbo; ///< Mandatory
		Vbo* tVbo; ///< Mandatory
		std::vector<Vbo*> texVbo; ///< Optional. Tex coords per channel
		std::vector<Vbo*> iVbo; ///< Mandatory. Indices VBO per LOD
		Vbo* wVbo; ///< Optional

		std::vector<uint> idsNum; ///< Indices count per LOD
		std::vector<uint> vertsNum; ///< Vertex count per LOD
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
