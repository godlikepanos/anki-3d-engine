#include "anki/renderer/Is.h"
#include "anki/renderer/Renderer.h"

#define BLEND_ENABLE true

namespace anki {

//==============================================================================
Is::Is(Renderer* r_)
	: RenderingPass(r_)
{}

//==============================================================================
Is::~Is()
{}

//==============================================================================
void Is::init(const RendererInitializer& /*initializer*/)
{
	// Load the passes
	//
	// XXX

	// Load the programs
	//

	// Ambient pass
	ambientPassSProg.load("shaders/IsAp.glsl");

	// point light
	pointLightSProg.load(ShaderProgramResource::createSrcCodeToCache(
		"shaders/IsLpGeneric.glsl", "#define POINT_LIGHT 1\n").c_str());

	// spot light no shadow
	spotLightNoShadowSProg.load(ShaderProgramResource::createSrcCodeToCache(
		"shaders/IsLpGeneric.glsl", "#define SPOT_LIGHT 1\n").c_str());

	// spot light w/t shadow
	std::string pps = std::string("#define SPOT_LIGHT 1\n#define SHADOW 1\n");
	if(/*sm.isPcfEnabled()*/ 1) // XXX
	{
		pps += "#define PCF 1\n";
	}
	spotLightShadowSProg.load(ShaderProgramResource::createSrcCodeToCache(
		"shaders/IsLpGeneric.glsl", pps.c_str()).c_str());
}

//==============================================================================
void Is::run()
{
	/// TODO
}

} // end namespace anki
