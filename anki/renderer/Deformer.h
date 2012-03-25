#ifndef ANKI_RENDERER_DEFORMER_H
#define ANKI_RENDERER_DEFORMER_H

#include "anki/resource/Resource.h"


namespace anki {


class ShaderProgram;
class SkinPatchNode;


/// SkinPatchNode deformer. It gets a SkinPatchNode and using transform
/// feedback it deforms its vertex attributes
class Deformer
{
	public:
		Deformer()
		{
			init();
		}
		~Deformer();

		void deform(SkinPatchNode& node, SkinPatchNode& subNode) const;

	private:
		ShaderProgramResourcePointer tfHwSkinningAllSProg;
		ShaderProgramResourcePointer tfHwSkinningPosSProg;

		void init();
};


} // end namespace


#endif
