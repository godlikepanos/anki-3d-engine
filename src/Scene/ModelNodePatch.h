#ifndef MODEL_NODE_PATCH_H
#define MODEL_NODE_PATCH_H

#include "Vao.h"
#include "Vbo.h"
#include "Mesh.h" // For the Vbos enum
#include "RsrcPtr.h"
#include "ModelPatch.h"
#include "Properties.h"
#include "SceneRenderable.h"


class Material;
class ModelNode;


/// A fragment of the ModelNode
class ModelNodePatch: public SceneRenderable
{
	/// VAO for MS and BS. All VBOs could be attached except for the vert weights
	PROPERTY_R(Vao, cpVao, getCpVao)

	/// VAO for depth passes. All VBOs could be attached except for the vert weights
	PROPERTY_R(Vao, dpVao, getDpVao)

	public:
		ModelNodePatch(const ModelNode& modelNode, const ModelPatch& modelPatch);

		/// @name Accessors
		/// @{
		const Material& getCpMtl() const {return modelPatchRsrc.getCpMtl();}
		const Material& getDpMtl() const {return modelPatchRsrc.getDpMtl();}
		const Transform& getWorldTransform() const;
		const ModelPatch& getModelPatchRsrc() const {return modelPatchRsrc;}
		uint getVertIdsNum() const {return modelPatchRsrc.getMesh().getVertIdsNum();}
		/// @}

	protected:
		const ModelNode& node; ///< Know your father
		const ModelPatch& modelPatchRsrc;

		static void createVao(const Material& material, const boost::array<const Vbo*, Mesh::VBOS_NUM>& vbos, Vao& vao);
};


#endif
