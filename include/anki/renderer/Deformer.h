#ifndef ANKI_RENDERER_DEFORMER_H
#define ANKI_RENDERER_DEFORMER_H

#include "anki/resource/Resource.h"

namespace anki {

class ProgramResource;
class SkinPatchNode;
class SkinNode;

/// @addtogroup renderer
/// @{

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
	ProgramResourcePointer tfHwSkinningAllSProg;

	/// Shader program that deforms only the position attribute
	ProgramResourcePointer tfHwSkinningPosSProg;

	void init();
};

/// @}

} // end namespace

#endif
