#ifndef MESH_H
#define MESH_H

#include "Math.h"
#include "Resource.h"
#include "RsrcPtr.h"
#include "Object.h"


class Material;
class MeshData;
class Vbo;


/// Mesh Resource. It contains the geometry packed in VBOs
class Mesh: public Resource, public Object
{
	public:
		/// Used in @ref vbos array
		enum Vbos
		{
			VBO_VERT_POSITIONS,
			VBO_VERT_NORMALS,
			VBO_VERT_TANGENTS,
			VBO_TEX_COORDS,
			VBO_VERT_INDECES,
			VBO_VERT_WEIGHTS,
			VBOS_NUM
		};

	PROPERTY_R(uint, vertIdsNum, getVertIdsNum)

	public:
		/// Default constructor
		Mesh();

		/// Does nothing
		~Mesh() {}

		/// Accessor
		const Vbo* getVbo(Vbos id) const {return vbos[id];}

		/// Implements @ref Resource::load
		void load(const char* filename);

	private:
		Vbo* vbos[VBOS_NUM]; ///< The vertex buffer objects

		/// Create the VBOs
		void createVbos(const MeshData& meshData);
};


#endif
