#ifndef MODEL_NODE_PATCH_H
#define MODEL_NODE_PATCH_H

#include <boost/array.hpp>
#include "Vao.h"
#include "Vbo.h"
#include "Mesh.h" // For the Vbos enum


/// A fragment of the ModelNode
class ModelNodePatch
{
	public:
		/// Transform feedback VBOs
		enum TfVbos
		{
			TF_VBO_POSITIONS,
			TF_VBO_NORMALS,
			TF_VBO_TANGENTS,
			TF_VBOS_NUM
		};

		Vbo& getTfVbo(TfVbos i) {return tfVbos[i];}

	private:
		const Model::SubModel& modelPatch;
		boost::array<Vbo, TF_VBOS_NUM> tfVbos;
		boost::array<Vbo*, Mesh::VBOS_NUM> vbos;
		Vao mainVao; ///< VAO for MS and BS
		Vao dpVao; ///< VAO for depth passes
		Vao tfVao; ///< VAO for transform feedback
};


#endif
