#include "SkinsDeformer.h"
#include "ShaderProg.h"


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void SkinsDeformer::init()
{
	//
	// Load the shaders
	//
	std::string all = ShaderProg::createSrcCodeToCache("shaders/TfHwSkinningGeneric.glsl",
	                                                   "#define NORMAL_ENABLED\n#define TANGENT_ENABLED\n",
	                                                   "pnt");

	std::string p = ShaderProg::createSrcCodeToCache("shaders/TfHwSkinningGeneric.glsl",
	                                                 "",
	                                                 "p");


	tfHwSkinningAllSProg.loadRsrc(all.c_str());
	tfHwSkinningPosSProg.loadRsrc(p.c_str());
}


//======================================================================================================================
// run                                                                                                                 =
//======================================================================================================================
void SkinsDeformer::run()
{

}
