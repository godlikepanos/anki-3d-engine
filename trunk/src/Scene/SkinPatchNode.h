#ifndef SKIN_PATCH_NODE_H
#define SKIN_PATCH_NODE_H

#include <boost/array.hpp>
#include "PatchNode.h"
#include "GfxApi/BufferObjects/Vbo.h"
#include "GfxApi/BufferObjects/Vao.h"


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

		/// @name Accessors
		/// @{
		GETTER_R(Vao, tfVao, getTfVao)
		const Vbo& getTfVbo(uint i) const {return tfVbos[i];}
		/// @}

		virtual void moveUpdate() {}
		virtual void frameUpdate(float /*prevUpdateTime*/, float /*crntTime*/) {}

	private:
		boost::array<Vbo, TFV_NUM> tfVbos; ///< VBOs that contain the deformed vertex attributes
		Vao tfVao; ///< For TF passes
};


#endif
