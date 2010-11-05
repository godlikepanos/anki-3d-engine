#ifndef MESH_H
#define MESH_H

#include "Math.h"
#include "Vbo.h"
#include "Vao.h"
#include "Resource.h"
#include "RsrcPtr.h"
#include "Object.h"


class Material;
class MeshData;


/// Mesh Resource. If the material name is empty then the mesh wont be rendered and no VBOs will be created
class Mesh: public Resource, public Object
{
	public:
		/// Used in @ref vao array
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
		RsrcPtr<Material> material; ///< Required. If empty then mesh not renderable
		Vao* vao; ///< Vertex array object
		Vao* depthVao; ///< Vertex array object for the depth material

		/// Default constructor
		Mesh(): Resource(RT_MESH), Object(NULL) {}

		/// Does nothing
		~Mesh() {}

		/// Implements @ref Resource::load
		void load(const char* filename);

		/// The mesh is renderable when the material is loaded
		bool isRenderable() const;

	private:
		Vbo* vbos[VBOS_NUM]; ///< The vertex buffer objects

		/// Create the VBOs
		void createVbos(const MeshData& meshData);

		/// Create a VAO. Called more than one
		void createVao(Vao* vao, Material& mtl);
};


#endif
