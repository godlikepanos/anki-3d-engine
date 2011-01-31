#ifndef MODEL_PATCH_NODE_H
#define MODEL_PATCH_NODE_H

#include "Vao.h"
#include "Vbo.h"
#include "Mesh.h" // For the Vbos enum
#include "RsrcPtr.h"
#include "ModelPatch.h"
#include "Properties.h"
#include "RenderableNode.h"


class Material;
class ModelNode;


/// A fragment of the ModelNode
class ModelPatchNode: public RenderableNode
{
	public:
		ModelPatchNode(const ModelNode& modelNode, const ModelPatch& modelPatch, ModelNode* parent);

		void init(const char*) {}

		/// @name Accessors
		/// @{
		const Material& getCpMtl() const {return rsrc.getCpMtl();}
		const Material& getDpMtl() const {return rsrc.getDpMtl();}
		const ModelPatch& getModelPatchRsrc() const {return rsrc;}
		GETTER_R(Vao, cpVao, getCpVao)
		GETTER_R(Vao, dpVao, getDpVao)
		uint getVertIdsNum() const {return rsrc.getMesh().getVertIdsNum();}
		/// @}

	protected:
		const ModelPatch& rsrc;
		Vao dpVao; /// VAO for depth passes. All VBOs could be attached except for the vert weights
		Vao cpVao; /// VAO for MS and BS. All VBOs could be attached except for the vert weights

		static void createVao(const Material& material, const boost::array<const Vbo*, Mesh::VBOS_NUM>& vbos, Vao& vao);
};


#endif
