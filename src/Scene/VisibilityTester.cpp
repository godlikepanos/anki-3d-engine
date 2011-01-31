#include <boost/foreach.hpp>
#include "VisibilityTester.h"
#include "Scene.h"
#include "ModelNode.h"
#include "ModelPatchNode.h"
#include "Material.h"
#include "Sphere.h"
#include "PointLight.h"
#include "SpotLight.h"


//======================================================================================================================
// CmpDistanceFromOrigin::operator()                                                                                   =
//======================================================================================================================
bool VisibilityTester::CmpDistanceFromOrigin::operator()(const RenderableNode* a, const RenderableNode* b) const
{
	return (a->getWorldTransform().origin - o).getLengthSquared() <
	       (b->getWorldTransform().origin - o).getLengthSquared();
}


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
VisibilityTester::VisibilityTester(const Scene& scene_):
	scene(scene_)
{}


//======================================================================================================================
// test                                                                                                                =
//======================================================================================================================
void VisibilityTester::test(const Camera& cam)
{
	//
	// Clean
	//
	msRenderables.clear();
	bsRenderables.clear();
	pointLights.clear();
	spotLights.clear();

	//
	// Collect the renderables
	//
	BOOST_FOREACH(const ModelNode* node, scene.getModelNodes())
	{
		// Skip if the ModeNode is not visible
		if(!test(*node, cam))
		{
			continue;
		}

		// If not test every patch individually
		BOOST_FOREACH(const ModelPatchNode* modelPatchNode, node->getModelPatchNodes())
		{
			// Test if visible by main camera
			if(test(*modelPatchNode, cam))
			{
				if(modelPatchNode->getCpMtl().isBlendingEnabled())
				{
					bsRenderables.push_back(modelPatchNode);
				}
				else
				{
					msRenderables.push_back(modelPatchNode);
				}
			}
		}
	}

	//
	// Sort the renderables from closest to the camera to the farthest
	//
	std::sort(msRenderables.begin(), msRenderables.end(), CmpDistanceFromOrigin(cam.getWorldTransform().origin));
	std::sort(bsRenderables.begin(), bsRenderables.end(), CmpDistanceFromOrigin(cam.getWorldTransform().origin));

	//
	// Collect the lights
	//
	BOOST_FOREACH(const Light* light, scene.getLights())
	{
		switch(light->getType())
		{
			// Point
			case Light::LT_POINT:
			{
				const PointLight& pointl = static_cast<const PointLight&>(*light);

				Sphere sphere(pointl.getWorldTransform().origin, pointl.getRadius());
				if(cam.insideFrustum(sphere))
				{
					pointLights.push_back(VisibleLight<PointLight>());
					pointLights.back().light = &pointl;
				}
				break;
			}
			// Spot
			case Light::LT_SPOT:
			{
				const SpotLight& spotl = static_cast<const SpotLight&>(*light);

				if(cam.insideFrustum(spotl.getCamera()))
				{
					spotLights.push_back(VisibleLight<SpotLight>());
					spotLights.back().light = &spotl;
				}
				break;
			}
		} // end switch
	} // end for all lights

	//
	// For all the spot lights get the visible renderables
	//
	BOOST_FOREACH(VisibleLight<SpotLight>& sl, spotLights)
	{
		// Skip if the light doesnt cast shadow
		if(!sl.light->castsShadow())
		{
			continue;
		}

		BOOST_FOREACH(const ModelNode* node, scene.getModelNodes())
		{
			// Skip if the ModeNode is not visible
			if(!test(*node, sl.light->getCamera()))
			{
				continue;
			}

			// If not test every patch individually
			BOOST_FOREACH(const ModelPatchNode* modelPatchNode, node->getModelPatchNodes())
			{
				// Skip if doesnt cast shadow
				if(!modelPatchNode->getCpMtl().isShadowCaster())
				{
					continue;
				}

				if(test(*modelPatchNode, sl.light->getCamera()))
				{
					sl.renderables.push_back(modelPatchNode);
				}
			} // end for all patches
		} // end for all model nodes

		std::sort(sl.renderables.begin(), sl.renderables.end(),
		          CmpDistanceFromOrigin(sl.light->getWorldTransform().origin));
	} // end for all spot lights
}


//======================================================================================================================
// test                                                                                                                =
//======================================================================================================================
template<typename Type>
bool VisibilityTester::test(const Type& tested, const Camera& cam)
{
	return cam.insideFrustum(tested.getBoundingShapeWSpace());
}
