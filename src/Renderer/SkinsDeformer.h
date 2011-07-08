#ifndef R_SKINS_DEFORMER_H
#define R_SKINS_DEFORMER_H

#include "Resources/RsrcPtr.h"


class ShaderProg;
class SkinPatchNode;


namespace R {


/// @todo
class SkinsDeformer
{
	public:
		enum TfShaderProgAttrib
		{
			TFSPA_POSITION,
			TFSPA_NORMAL,
			TFSPA_TANGENT,
			TFSPA_VERT_WEIGHT_BONES_NUM,
			TFSPA_VERT_WEIGHT_BONE_IDS,
			TFSPA_VERT_WEIGHT_WEIGHTS
		};

		void init();
		void deform(SkinPatchNode& node);

	private:
		RsrcPtr<ShaderProg> tfHwSkinningAllSProg;
		RsrcPtr<ShaderProg> tfHwSkinningPosSProg;
};


} // end namespace


#endif
