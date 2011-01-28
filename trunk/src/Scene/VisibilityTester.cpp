#include "VisibilityTester.h"
#include "Scene.h"
#include "ModelNode.h"
#include "ModelNodePatch.h"
#include "Material.h"
#include "Sphere.h"
#include "PointLight.h"
#include "SpotLight.h"


//======================================================================================================================
// CmpLength::operator()                                                                                               =
//======================================================================================================================
inline bool VisibilityTester::CmpLength::operator()(const SceneRenderable* a, const SceneRenderable* b) const
{
	return (a->getWorldTransform().origin - o).getLengthSquared() < (b->getWorldTransform().origin - o).getLengthSquared();
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
	// Collect the lights
	//
	Scene::Types<Light>::ConstIterator itl = scene.getLights().begin();
	for(; itl != scene.getLights().end(); itl++)
	{
		const Light& light = *(*itl);

		// Point
		switch(light.getType())
		{
			case Light::LT_POINT:
			{
				const PointLight& pointl = static_cast<const PointLight&>(light);

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
				const SpotLight& spotl = static_cast<const SpotLight&>(light);

				if(cam.insideFrustum(spotl.getCamera()))
				{
					spotLights.push_back(VisibleLight<SpotLight>());
					spotLights.back().light = &spotl;
				}
				break;
			}
		}
	}

	//
	// Collect the renderables
	//
	Scene::Types<ModelNode>::ConstIterator it = scene.getModelNodes().begin();
	for(; it != scene.getModelNodes().end(); it++)
	{
		boost::ptr_vector<ModelNodePatch>::const_iterator it2 = (*it)->getModelNodePatches().begin();
		for(; it2 != (*it)->getModelNodePatches().end(); it2++)
		{
			const ModelNodePatch& modelNodePatch = *it2;

			// First check if its rendered by a light
			Types<VisibleLight<SpotLight> >::Iterator itsl = spotLights.begin();
			for(; itsl != spotLights.end(); itsl++)
			{
				const SpotLight& spotLight = *(itsl->light);
				if(test(modelNodePatch, spotLight.getCamera()))
				{
					itsl->renderables.push_back(&modelNodePatch);
				}
			}


			/// @todo Perform real tests
			if(test(modelNodePatch, cam))
			{
				if(modelNodePatch.getCpMtl().isBlendingEnabled())
				{
					bsRenderables.push_back(&modelNodePatch);
				}
				else
				{
					msRenderables.push_back(&modelNodePatch);
				}
			}
		}
	}

	//
	// Sort the renderables from closest to the camera to the farthest
	//
	std::sort(msRenderables.begin(), msRenderables.end(), CmpLength(cam.getWorldTransform().origin));
	std::sort(bsRenderables.begin(), bsRenderables.end(), CmpLength(cam.getWorldTransform().origin));

	//
	// Short the light's renderables
	//
	Types<VisibleLight<SpotLight> >::Iterator itsl = spotLights.begin();
	for(; itsl != spotLights.end(); itsl++)
	{
		std::sort(itsl->renderables.begin(), itsl->renderables.end(), CmpLength(itsl->light->getWorldTransform().origin));
	}
}
