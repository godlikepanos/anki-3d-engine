#ifndef SKIN_PATCH_NODE_H
#define SKIN_PATCH_NODE_H

#include <boost/array.hpp>
#include "PatchNode.h"
#include "Vbo.h"
#include "Vao.h"


class SkinNode;


/// A fragment of the SkinNode
class SkinPatchNode: public PatchNode
{
	public:
		enum TransformFeedbackVbo
		{
			TFV_POSITIONS,
			TFV_NORMALS,
			TFV_TANGENTS,
			TFV_NUM
		};

		SkinPatchNode(const ModelPatch& modelPatch, SkinNode* parent);

	public:
		boost::array<Vbo, TFV_NUM> tfVbos; ///< VBOs that contain the deformed vertex attributes
		Vao tfVao; ///< For TF passes
};


#endif
