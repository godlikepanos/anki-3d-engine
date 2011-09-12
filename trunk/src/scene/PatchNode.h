#ifndef PATCH_NODE_H
#define PATCH_NODE_H

#include "gl/Vao.h"
#include "gl/Vbo.h"
#include "rsrc/Mesh.h" // For the Vbos enum
#include "rsrc/RsrcPtr.h"
#include "rsrc/ModelPatch.h"
#include "RenderableNode.h"
#include "MaterialRuntime.h"
#include <boost/scoped_ptr.hpp>
#include <boost/array.hpp>


class Material;


/// Inherited by ModelPatchNode and SkinPatchNode. It contains common code,
/// the derived classes are responsible to initialize the VAOs
class PatchNode: public RenderableNode
{
	public:
		typedef boost::array<const Vbo*, Mesh::VBOS_NUM> VboArray;

		static bool classof(const SceneNode* x);

		/// @name Accessors
		/// @{

		/// Implements RenderableNode::getVao
		const Vao& getVao(PassType p) const {return vaos[p];}

		/// Implements RenderableNode::getVertIdsNum
		uint getVertIdsNum() const {return rsrc.getMesh().getVertIdsNum();}

		/// Implements RenderableNode::getMaterial
		const Material& getMaterial() const {return rsrc.getMaterial();}

		/// Implements RenderableNode::getMaterialRuntime
		MaterialRuntime& getMaterialRuntime() {return *mtlRun;}

		/// Implements RenderableNode::getMaterialRuntime
		const MaterialRuntime& getMaterialRuntime() const {return *mtlRun;}

		const ModelPatch& getModelPatchRsrc() const {return rsrc;}
		/// @}

	protected:
		/// The sub-resource
		const ModelPatch& rsrc;

		/// The VAOs. All VBOs could be attached except for the vertex weights
		boost::array<Vao, PASS_TYPES_NUM> vaos;

		boost::scoped_ptr<MaterialRuntime> mtlRun; ///< Material runtime

		PatchNode(ClassId cid, const ModelPatch& modelPatch, ulong flags,
			SceneNode& parent);

		/// Create a VAO given a material and an array of VBOs
		/// The location of the uniform variables are hard coded. See
		/// MaterialVertex.glsl
		static void createVao(const Material& material,
			const VboArray& vbos,
			Vao& vao);
};


inline bool PatchNode::classof(const SceneNode* x)
{
	return x->getClassId() == CID_PATCH_NODE ||
		x->getClassId() == CID_MODEL_PATCH_NODE ||
		x->getClassId() == CID_SKIN_PATCH_NODE;
}


#endif
