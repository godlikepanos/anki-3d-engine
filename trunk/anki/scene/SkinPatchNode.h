#ifndef ANKI_SCENE_SKIN_NODE_SKIN_PATCH_NODE_H
#define ANKI_SCENE_SKIN_NODE_SKIN_PATCH_NODE_H

#include <boost/array.hpp>
#include "anki/scene/PatchNode.h"
#include "anki/gl/Vbo.h"
#include "anki/gl/Vao.h"


namespace anki {


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

		/// See TfHwSkinningGeneric.glsl for the locations
		enum TfShaderProgAttribLoc
		{
			POSITION_LOC,
			NORMAL_LOC,
			TANGENT_LOC,
			VERT_WEIGHT_BONES_NUM_LOC,
			VERT_WEIGHT_BONE_IDS_LOC,
			VERT_WEIGHT_WEIGHTS_LOC
		};

		SkinPatchNode(const ModelPatch& modelPatch, SkinNode& parent);

		void init(const char*)
		{}

		/// @name Accessors
		/// @{
		const Vao& getTfVao() const
		{
			return tfVao;
		}

		const Vbo& getTfVbo(uint i) const
		{
			return tfVbos[i];
		}

		const Vao& getVao(const PassLevelKey& k) const
		{
			return *vaosHashMap.at(k);
		}
		/// @}

	private:
		ModelPatch::VaosContainer vaos;
		ModelPatch::PassLevelToVaoHashMap vaosHashMap;

		boost::array<Vbo, TFV_NUM> tfVbos;
		Vao tfVao; ///< For TF passes
};


} // end namespace


#endif
