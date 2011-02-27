#ifndef PATCH_NODE_H
#define PATCH_NODE_H


#include "Vao.h"
#include "Vbo.h"
#include "Mesh.h" // For the Vbos enum
#include "RsrcPtr.h"
#include "ModelPatch.h"
#include "RenderableNode.h"


class Material;


/// Inherited by ModelPatchNode and SkinPatchNode. It contains common code, the derived classes are responsible to
/// initialize the VAOs
class PatchNode: public RenderableNode
{
	public:
		PatchNode(const ModelPatch& modelPatch, SceneNode* parent);

		/// Do nothing
		void init(const char*) {}

		/// @name Accessors
		/// @{
		const Material& getCpMtl() const {return rsrc.getCpMtl();}
		const Material& getDpMtl() const {return rsrc.getDpMtl();}
		const ModelPatch& getModelPatchRsrc() const {return rsrc;}
		const Vao& getCpVao() const {return cpVao;}
		const Vao& getDpVao() const {return dpVao;}
		uint getVertIdsNum() const {return rsrc.getMesh().getVertIdsNum();}
		/// @}

	protected:
		const ModelPatch& rsrc;
		Vao dpVao; /// VAO for depth passes. All VBOs could be attached except for the vert weights
		Vao cpVao; /// VAO for MS and BS. All VBOs could be attached except for the vert weights

		/// Create a VAO given a material and an array of VBOs
		static void createVao(const Material& material, const boost::array<const Vbo*, Mesh::VBOS_NUM>& vbos, Vao& vao);
};


inline PatchNode::PatchNode(const ModelPatch& modelPatch, SceneNode* parent):
	RenderableNode(modelPatch.getMesh().getVisibilityShape(), parent),
	rsrc(modelPatch)
{}


#endif
