#ifndef MESH_H
#define MESH_H

#include "Math.h"
#include "Vbo.h"
#include "Vao.h"
#include "Resource.h"
#include "RsrcPtr.h"


class Material;
class MeshData;


/// Mesh Resource. If the material name is empty then the mesh wont be rendered and no VBOs will be created
class Mesh: public Resource
{
	public:
		/// The VBOs in a structure
		struct Vbos
		{
			Vbo* vertCoords;
			Vbo* vertNormals;
			Vbo* vertTangents;
			Vbo* texCoords;
			Vbo* vertIndeces;
			Vbo* vertWeights;

			Vbos();
		};

	PROPERTY_R(uint, vertIdsNum, getVertIdsNum)

	public:
		RsrcPtr<Material> material; ///< Required. If empty then mesh not renderable
		Vbos vbos; ///< The vertex buffer objects
		Vao vao; ///< Vertex array object
		Vao depthVao; ///< Vertex array object for the depth material

		Mesh(): Resource(RT_MESH) {}
		~Mesh() {}

		/// Implements @ref Resource::load
		void load(const char* filename);

		/// The mesh is renderable when the material is loaded
		bool isRenderable() const;

	private:
		void createVbos(const MeshData& meshData);
		void createVao(Vao& vao, Material& mtl);
};


inline Mesh::Vbos::Vbos():
	vertCoords(NULL),
	vertNormals(NULL),
	vertTangents(NULL),
	texCoords(NULL),
	vertIndeces(NULL),
	vertWeights(NULL)
{}


#endif
