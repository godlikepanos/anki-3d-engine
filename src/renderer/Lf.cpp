#include "anki/renderer/Lf.h"
#include "anki/renderer/Renderer.h"
#include "anki/scene/SceneGraph.h"
#include "anki/scene/Movable.h"
#include "anki/scene/Camera.h"
#include "anki/scene/Light.h"

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

	// Retrive some things
	SceneGraph& scene = r->getSceneGraph();
	Camera& cam = scene.getActiveCamera();
	VisibilityTestResults& vi = *cam.getVisibilityTestResults();

	// Iterate the lights and update the UBO
	Array<Flare, 256> flareBuff; // XXX 256?
	ANKI_ASSERT(
		maxLensFlareCount * maxLightsWidthFlaresCount < flareBuff.size());

	U count = 0;

	for(auto it = vi.getLightsBegin(); it != vi.getLightsEnd(); ++it)
	{
		SceneNode& sn = *(*it).node;
		ANKI_ASSERT(sn.getLight());
		Light& light = *sn.getLight();

		if(!light.hasLensFlare())
		{
			continue;
		}

		// Transform
		Vec3 posworld = sn.getMovable()->getWorldTransform().getOrigin();
		Vec4 posclip = cam.getViewProjectionMatrix() * Vec4(posworld, 1.0);
		Vec2 posndc = (posclip.xyz() / posclip.w()).xy();

		Vec2 dir = -posndc;
		F32 len = dir.getLength() * 2.0;
		dir /= len;

		const Texture& tex = light.getLensFlareTexture();
		const U depth = tex.getDepth();

		for(U d = 0; d < depth; d++)
		{
			flareBuff[count].pos = posndc + dir * (len * (d / (F32)depth));
			flareBuff[count].scale = Vec2(0.1 * 3.0, r->getAspectRatio() * 0.1);
			++count;
		}
	}

	flareDataUbo.write(&flareBuff[0], 0, sizeof(Flare) * count);

	// Draw
	drawProg->bind();
	drawProg->findUniformVariable("images").set(*tex);
	flareDataUbo.setBinding(0);

	GlStateSingleton::get().enable(GL_BLEND);
	GlStateSingleton::get().setBlendFunctions(GL_ONE, GL_ONE);
	r->drawQuadInstanced(tex->getDepth());
}

} // end namespace anki
