#ifndef SKINS_DEFORMER_H
#define SKINS_DEFORMER_H

#include "RsrcPtr.h"


class ShaderProg;


///
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
		void run();

	private:
		RsrcPtr<ShaderProg> tfHwSkinningAllSProg;
		RsrcPtr<ShaderProg> tfHwSkinningPosSProg;
};


#endif
