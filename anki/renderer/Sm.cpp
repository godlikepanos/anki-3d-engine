#include <boost/foreach.hpp>
#include "anki/renderer/Sm.h"
#include "anki/renderer/Renderer.h"
#include "anki/core/App.h"
#include "anki/scene/Scene.h"
#include "anki/resource/LightRsrc.h"
#include "anki/scene/Camera.h"
#include "anki/scene/Light.h"
#include "anki/scene/SpotLight.h"
#include "anki/renderer/RendererInitializer.h"


namespace anki {


//==============================================================================
// init                                                                        =
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
	for(uint i = 1; i < levels.size(); i++)
	{
		initLevel(levels[i - 1].resolution / 2,
			levels[i - 1].distance * 2.0,
			bilinearEnabled,
			levels[i]);
	}
}


//==============================================================================
// initLevel                                                                   =
//==============================================================================
void Sm::initLevel(uint resolution, float distance, bool bilinear, Level& level)
{
	level.resolution = resolution;
	level.distance = distance;
	level.bilinear = bilinear;

	try
	{
		// create FBO
		level.fbo.create();
		level.fbo.bind();

		// texture
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

		// inform the we wont write to color buffers
		level.fbo.setNumOfColorAttachements(0);

		// attach the texture
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
			GL_TEXTURE_2D, level.shadowMap.getGlId(), 0);

		// test if success
		level.fbo.checkIfGood();

		// unbind
		level.fbo.unbind();
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Cannot create shadowmapping "
			"FBO") << e;
	}
}


//==============================================================================
// run                                                                         =
//==============================================================================
void Sm::run(Light& light, float distance)
{
	if(!enabled)
	{
		return;
	}

	ANKI_ASSERT(light.getVisibleMsRenderableNodes().size() > 0);

	//
	// Determine the level
	//
	BOOST_FOREACH(Level& level, levels)
	{
		crntLevel = &level;
		if(distance < level.distance)
		{
			break;
		}
	}

	//
	// Render
	//

	// FBO
	crntLevel->fbo.bind();

	// set GL
	GlStateMachineSingleton::get().setViewport(0, 0,
		crntLevel->resolution, crntLevel->resolution);
	glClear(GL_DEPTH_BUFFER_BIT);

	// disable color & blend & enable depth test
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	GlStateMachineSingleton::get().enable(GL_DEPTH_TEST, true);
	GlStateMachineSingleton::get().enable(GL_BLEND, false);

	// for artifacts
	glPolygonOffset(2.0, 2.0); // keep the values as low as possible!!!!
	GlStateMachineSingleton::get().enable(GL_POLYGON_OFFSET_FILL);

	// render all
	BOOST_FOREACH(RenderableNode* node, light.getVisibleMsRenderableNodes())
	{
		switch(light.getLightType())
		{
			case Light::LT_SPOT:
			{
				const SpotLight& sl = static_cast<const SpotLight&>(light);
				r.getSceneDrawer().renderRenderableNode(sl.getCamera(),
					1, *node);
				break;
			}

			default:
				ANKI_ASSERT(0);
		}
	}

	// restore GL
	GlStateMachineSingleton::get().disable(GL_POLYGON_OFFSET_FILL);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	// FBO
	crntLevel->fbo.unbind();
}


} // end namespace
