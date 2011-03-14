#include "MtlUserDefinedVar.h"
#include "Texture.h"


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
MtlUserDefinedVar::MtlUserDefinedVar(const SProgUniVar& sProgVar, const char* texFilename):
	sProgVar(sProgVar)
{
	ASSERT(sProgVar.getGlDataType() == GL_SAMPLER_2D);
	data.texture.loadRsrc(texFilename);
}
