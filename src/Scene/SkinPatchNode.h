#ifndef SKIN_PATCH_NODE_H
#define SKIN_PATCH_NODE_H

#include "Vao.h"
#include "Vbo.h"
#include "Mesh.h" // For the Vbos enum
#include "RsrcPtr.h"
#include "Properties.h"
#include "RenderableNode.h"


class Material;


/// A fragment of the SkinNode
class SkinPatchNode: public RenderableNode
{
	/*public:
		SkinPatchNode(const ModelNode& modelNode, const ModelPatch& modelPatch);

		/// @name Accessors
		/// @{
		const Material& getCpMtl() const {return modelPatchRsrc.getCpMtl();}
		const Material& getDpMtl() const {return modelPatchRsrc.getDpMtl();}
		const ModelPatch& getModelPatchRsrc() const {return modelPatchRsrc;}
		uint getVertIdsNum() const {return rsrc.getMesh().getVertIdsNum();}
		/// @}

	protected:
		const SkinNode& node; ///< Know your father
		const ModelPatch& rsrc;
		Vao dpVao;
		Vao cpVao;*/
};


#endif
