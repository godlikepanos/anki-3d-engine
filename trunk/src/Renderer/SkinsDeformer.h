#ifndef SKINS_DEFORMER_H
#define SKINS_DEFORMER_H

#include "RsrcPtr.h"


class SkinsDeformer
{
	public:
		void init();
		void run();

	private:
		RsrcPtr<ShaderProg> tfHwSkinningAllSProg;
		RsrcPtr<ShaderProg> tfHwSkinningPosNormSProg;
		RsrcPtr<ShaderProg> tfHwSkinningPosTangentSProg;
};


#endif
