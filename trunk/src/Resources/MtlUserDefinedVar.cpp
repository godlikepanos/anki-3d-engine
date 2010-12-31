#include "MtlUserDefinedVar.h"


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
MtlUserDefinedVar::MtlUserDefinedVar(const SProgUniVar& sProgVar, const char* texFilename):
	sProgVar(sProgVar)
{
	texture.loadRsrc(texFilename);
}
