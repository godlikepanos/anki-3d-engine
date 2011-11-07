#ifndef ANKI_RENDERER_DEFORMER_H
#define ANKI_RENDERER_DEFORMER_H

#include "anki/resource/Resource.h"


namespace anki {


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
		ShaderProgramResourcePointer tfHwSkinningAllSProg;
		ShaderProgramResourcePointer tfHwSkinningPosSProg;

		void init();
};


} // end namespace


#endif
