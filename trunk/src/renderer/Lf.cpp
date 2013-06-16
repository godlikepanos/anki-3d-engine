#include "anki/renderer/Lf.h"
#include "anki/renderer/Renderer.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/Movable.h"
#include "anki/scene/Camera.h"

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

//==============================================================================
struct Flare
{
	Vec2 pos; ///< Position in NDC
	Vec2 scale; ///< Scale of the quad
	//F32 alpha; ///< Difference in alpha
};

//==============================================================================
// Lf                                                                          =
//==============================================================================

//==============================================================================
Lf::~Lf()
{}

//==============================================================================
void Lf::init(const RendererInitializer& initializer)
{
	enabled = initializer.pps.lf.enabled;
	if(!enabled)
	{
		return;
	}

	tex.load("data/textures/lens_flare/flares0.ankitex");

	maxLensFlareCount = initializer.pps.lf.maxLensFlareCount;
	maxLightsWidthFlaresCount = initializer.pps.lf.maxLightsWidthFlaresCount;

	// Load the shader
	std::string pps = "#define MAX_FLARES " 
		+ std::to_string(initializer.pps.lf.maxLensFlareCount) + "\n";
	std::string fname = ShaderProgramResource::createSrcCodeToCache(
		"shaders/PpsLfPass.glsl", pps.c_str());
	drawProg.load(fname.c_str());

	// Init UBO
	flareDataUbo.create(
		sizeof(Flare) * maxLensFlareCount * maxLightsWidthFlaresCount,
		nullptr);
}

//==============================================================================
void Lf::run()
{
	ANKI_ASSERT(enabled);

	// Update the UBO
	{
		SceneGraph& scene = r->getSceneGraph();
		const Camera& cam = scene.getActiveCamera();

		SceneNode& sn = scene.findSceneNode("vase_plight0");
		Vec4 snPos = Vec4(sn.getMovable()->getWorldTransform().getOrigin(), 1.0);
		snPos = cam.getViewProjectionMatrix() * snPos;
		Vec2 posNdc = snPos.xy() / snPos.w();

		Flare flare;
		flare.pos = posNdc;
		flare.scale = Vec2(0.2 * 3.0, r->getAspectRatio() * 0.2);
		//flare.alpha = 0.2;

		flareDataUbo.write(&flare, 0, sizeof(Flare));
	}
	
	drawProg->bind();
	drawProg->findUniformVariable("images").set(*tex);
	flareDataUbo.setBinding(0);

	/*GlStateSingleton::get().enable(GL_BLEND);
	GlStateSingleton::get().setBlendFunctions(GL_ONE, GL_ONE);*/
	r->drawQuad();
}

} // end namespace anki
