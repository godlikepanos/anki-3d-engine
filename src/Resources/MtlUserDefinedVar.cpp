#include "MtlUserDefinedVar.h"
#include "Texture.h"


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
MtlUserDefinedVar::MtlUserDefinedVar(const SProgUniVar& sProgVar, const char* texFilename):
	sProgVar(sProgVar)
{
	texture.loadRsrc(texFilename);
}
