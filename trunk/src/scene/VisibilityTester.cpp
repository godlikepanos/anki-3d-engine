#include "anki/scene/VisibilityTester.h"
#include "anki/scene/Scene.h"
#include "anki/scene/Camera.h"
#include "anki/scene/Renderable.h"
#include "anki/scene/Light.h"

namespace anki {

//==============================================================================
VisibilityTester::~VisibilityTester()
{}

//==============================================================================
void VisibilityTester::test(Frustumable& ref, Scene& scene)
{
	VisibilityInfo& vinfo = ref.getVisibilityInfo();
	vinfo.renderables.clear();
	vinfo.lights.clear();

	for(auto it = scene.getAllNodesBegin(); it != scene.getAllNodesEnd(); it++)
	{
		SceneNode* node = *it;

		Frustumable* fr = node->getFrustumable();
		// Wont check the same
		if(&ref == fr)
		{
			continue;
		}

		Spatial* sp = node->getSpatial();
		if(!sp)
		{
			continue;
		}

		if(!ref.insideFrustum(*sp))
		{
			continue;
		}

		sp->enableFlag(Spatial::SF_VISIBLE);

		Renderable* r = node->getRenderable();
		if(r)
		{
			vinfo.renderables.push_back(node);
		}

		Light* l = node->getLight();
		if(l)
		{
			vinfo.lights.push_back(node);

			if(l->getShadowEnabled() && fr)
			{
				testLight(*l, scene);
			}
		}
	}
}

//==============================================================================
void VisibilityTester::testLight(Light& light, Scene& scene)
{
	Frustumable& ref = *light.getFrustumable();
	ANKI_ASSERT(&ref != nullptr);

	VisibilityInfo& vinfo = ref.getVisibilityInfo();
	vinfo.renderables.clear();
	vinfo.lights.clear();

	for(auto it = scene.getAllNodesBegin(); it != scene.getAllNodesEnd(); it++)
	{
		SceneNode* node = *it;

		Frustumable* fr = node->getFrustumable();
		// Wont check the same
		if(&ref == fr)
		{
			continue;
		}

		Spatial* sp = node->getSpatial();
		if(!sp)
		{
			continue;
		}

		if(!ref.insideFrustum(*sp))
		{
			continue;
		}

		sp->enableFlag(Spatial::SF_VISIBLE);

		Renderable* r = node->getRenderable();
		if(r)
		{
			vinfo.renderables.push_back(node);
		}
	}
}

} // end namespace anki
