#ifndef ANKI_RENDERER_DEFORMER_H
#define ANKI_RENDERER_DEFORMER_H

#include "anki/resource/Resource.h"

namespace anki {

class ShaderProgramResource;
class SkinPatchNode;
class SkinNode;

/// SkinPatchNode deformer
///
/// It gets a SkinPatchNode and using transform feedback it deforms its vertex 
/// attributes
class Deformer
{
public:
	Deformer()
	{
		init();
	}
	~Deformer();

	void deform(SkinNode& node, SkinPatchNode& subNode) const;

private:
	/// Shader program that deforms all attribs
	ShaderProgramResourcePointer tfHwSkinningAllSProg;

	/// Shader program that deforms only the position attribute
	ShaderProgramResourcePointer tfHwSkinningPosSProg;

	void init();
};

} // end namespace

#endif
