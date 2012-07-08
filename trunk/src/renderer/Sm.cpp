#include "anki/renderer/Sm.h"
#include "anki/renderer/Renderer.h"
#include "anki/core/App.h"
#include "anki/resource/LightRsrc.h"
#include "anki/scene/Scene.h"
#include "anki/scene/Camera.h"
#include "anki/scene/Light.h"

namespace anki {

//==============================================================================
void Sm::init(const RendererInitializer& initializer)
{
	enabled = initializer.is.sm.enabled;

	if(!enabled)
	{
		return;
	}

	pcfEnabled = initializer.is.sm.pcfEnabled;
	bilinearEnabled = initializer.is.sm.bilinearEnabled;
	resolution = initializer.is.sm.resolution;
	level0Distance = initializer.is.sm.level0Distance;

	// Init the levels
	initLevel(resolution, level0Distance, bilinearEnabled, levels[0]);
	for(uint32_t i = 1; i < levels.size(); i++)
	{
		initLevel(levels[i - 1].resolution / 2,
			levels[i - 1].distance * 2.0,
			bilinearEnabled,
			levels[i]);
	}
}

//==============================================================================
void Sm::initLevel(uint32_t resolution, float distance, bool bilinear,
	Level& level)
{
	level.resolution = resolution;
	level.distance = distance;
	level.bilinear = bilinear;

	try
	{
		// Init texture
		Renderer::createFai(level.resolution, level.resolution,
			GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT, level.shadowMap);

		if(level.bilinear)
		{
			level.shadowMap.setFiltering(Texture::TFT_LINEAR);
		}
		else
		{
			level.shadowMap.setFiltering(Texture::TFT_NEAREST);
		}
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,
			GL_COMPARE_R_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		/*If you dont want to use the FFP for comparing the shadowmap
		(the above two lines) then you can make the comparison
		inside the glsl shader. The GL_LEQUAL means that:
		shadow = (R <= Dt) ? 1.0 : 0.0; . The R is given by:
		R = _tex_coord2.z/_tex_coord2.w; and the
		Dt = shadow2D(shadow_depth_map, _shadow_uv).r (see lp_generic.frag).
		Hardware filters like GL_LINEAR cannot be applied.*/

		// create FBO
		level.fbo.create();
		level.fbo.setOtherAttachment(GL_DEPTH_ATTACHMENT, level.shadowMap);
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Cannot create shadowmapping FBO") << e;
	}
}

//==============================================================================
void Sm::run(Light& light, float distance)
{
	if(!enabled || !light.getShadowEnabled()
		|| light.getLightType() == Light::LT_POINT)
	{
		return;
	}

	// Determine the level
	//
	for(Level& level : levels)
	{
		crntLevel = &level;
		if(distance < level.distance)
		{
			break;
		}
	}

	// Render
	//

	// FBO
	crntLevel->fbo.bind();

	// set GL
	GlStateSingleton::get().setViewport(0, 0,
		crntLevel->resolution, crntLevel->resolution);
	glClear(GL_DEPTH_BUFFER_BIT);

	// disable color & blend & enable depth test
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	GlStateSingleton::get().enable(GL_DEPTH_TEST);
	GlStateSingleton::get().disable(GL_BLEND);

	// for artifacts
	glPolygonOffset(2.0, 2.0); // keep the values as low as possible!!!!
	GlStateSingleton::get().enable(GL_POLYGON_OFFSET_FILL);

	// Visibility tests
	SpotLight& slight = static_cast<SpotLight&>(light);
	VisibilityInfo vi;
	VisibilityTester vt;
	vt.test(slight, r->getScene(), vi);

	// render all
	for(auto it = vi.getRenderablesBegin(); it != vi.getRenderablesEnd(); ++it)
	{
		r->getSceneDrawer().render(r->getScene().getActiveCamera(), 1, *(*it));
	}

	// restore GL
	GlStateSingleton::get().disable(GL_POLYGON_OFFSET_FILL);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}


} // end namespace
