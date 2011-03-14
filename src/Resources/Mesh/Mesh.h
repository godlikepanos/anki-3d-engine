#ifndef MESH_H
#define MESH_H

#include <boost/array.hpp>
#include "Math.h"
#include "RsrcPtr.h"
#include "Vbo.h"
#include "Obb.h"


class MeshData;


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

		/// Default constructor
		Mesh() {}

		/// Does nothing
		~Mesh() {}

		/// @name Accessors
		/// @{
		const Vbo& getVbo(Vbos id) const {return vbos[id];}
		uint getVertIdsNum() const {return vertIdsNum;}
		const Obb& getVisibilityShape() const {return visibilityShape;}
		/// @}

		/// Implements @ref Resource::load
		void load(const char* filename);

		/// @name Ask for geometry properties
		/// @{
		bool hasTexCoords() const {return vbos[VBO_TEX_COORDS].isCreated();}
		bool hasVertWeights() const {return vbos[VBO_VERT_WEIGHTS].isCreated();}
		bool hasNormalsAndTangents() const;
		/// @}

	private:
		boost::array<Vbo, VBOS_NUM> vbos; ///< The vertex buffer objects
		uint vertIdsNum; ///< The number of vertex IDs
		Obb visibilityShape;

		/// Create the VBOs
		void createVbos(const MeshData& meshData);
};


inline bool Mesh::hasNormalsAndTangents() const
{
	return vbos[VBO_VERT_NORMALS].isCreated() && vbos[VBO_VERT_TANGENTS].isCreated();
}


#endif
