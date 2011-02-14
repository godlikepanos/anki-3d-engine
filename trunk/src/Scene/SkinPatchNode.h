#ifndef SKIN_PATCH_NODE_H
#define SKIN_PATCH_NODE_H

#include <boost/array.hpp>
#include "PatchNode.h"
#include "Vbo.h"


class SkinNode;


/// A fragment of the SkinNode
class SkinPatchNode: public PatchNode
{
	public:
		enum TransformFeedbackVbo
		{
			TFV_POSITIONS,
			TFV_NORMALS,
			TFV_TANGENTS
		};

		SkinPatchNode(const ModelPatch& modelPatch, SkinNode* parent);

	public:
		boost::array<Vbo, 3> tfVbos; ///< VBOs that contain the deformed vertex attributes
};


#endif
