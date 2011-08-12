#ifndef SKIN_PATCH_NODE_DEFORMER_H
#define SKIN_PATCH_NODE_DEFORMER_H

#include "Resources/RsrcPtr.h"


class ShaderProgram;
class SkinPatchNode;


/// XXX
class SkinPatchNodeDeformer
{
	public:
		SkinPatchNodeDeformer();
		~SkinPatchNodeDeformer();

		void deform(SkinPatchNode& node) const;

	private:
		RsrcPtr<ShaderProgram> tfHwSkinningAllSProg;
		RsrcPtr<ShaderProgram> tfHwSkinningPosSProg;

		void init();
};


#endif
