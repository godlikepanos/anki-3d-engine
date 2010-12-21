#include "SkinsDeformer.h"
#include "ShaderProg.h"


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void SkinsDeformer::init()
{
	std::string all = ShaderProg::createSrcCodeToCache("shaders/TfHwSkinningGeneric.glsl",
	                                                   "#define NORMAL_ENABLED\n#define TANGENT_ENABLED\n",
	                                                   "pnt");

	std::string pn = ShaderProg::createSrcCodeToCache("shaders/TfHwSkinningGeneric.glsl",
	                                                  "#define NORMAL_ENABLED\n",
	                                                  "pn");

	std::string pt = ShaderProg::createSrcCodeToCache("shaders/TfHwSkinningGeneric.glsl",
	                                                  "\n#define TANGENT_ENABLED\n",
	                                                  "pt");


	tfHwSkinningAllSProg.loadRsrc(all.c_str());
	tfHwSkinningPosNormSProg.loadRsrc(pn.c_str());
	tfHwSkinningPosTangentSProg.loadRsrc(pt.c_str());
}
