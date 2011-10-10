#ifndef DEFORMER_H
#define DEFORMER_H

#include "resource/RsrcPtr.h"


class ShaderProgram;
class SkinPatchNode;


class MainRenderer;


/// SkinPatchNode deformer. It gets a SkinPatchNode and using transform
/// feedback it deforms its vertex attributes
class Deformer
{
	public:
		Deformer(const MainRenderer& mainR);
		~Deformer();

		void deform(SkinPatchNode& node) const;

	private:
		const MainRenderer& mainR; ///< Know your father
		RsrcPtr<ShaderProgram> tfHwSkinningAllSProg;
		RsrcPtr<ShaderProgram> tfHwSkinningPosSProg;

		void init();
};


#endif
