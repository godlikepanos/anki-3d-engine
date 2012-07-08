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
void VisibilityTester::test(Frustumable& cam, Scene& scene,
	VisibilityInfo& vinfo)
{
	vinfo.renderables.clear();
	vinfo.lights.clear();

	for(auto it = scene.getAllNodesBegin(); it != scene.getAllNodesEnd(); it++)
	{
		SceneNode* node = *it;

		Frustumable* fr = node->getFrustumable();
		// Wont check the same
		if(&cam == fr)
		{
			continue;
		}

		Spatial* sp = node->getSpatial();
		if(!sp)
		{
			continue;
		}

		if(!cam.insideFrustum(*sp))
		{
			sp->disableFlag(Spatial::SF_VISIBLE);
			continue;
		}

		sp->enableFlag(Spatial::SF_VISIBLE);

		Renderable* r = node->getRenderable();
		if(r)
		{
			vinfo.renderables.push_back(r);
		}

		Light* l = node->getLight();
		if(l)
		{
			vinfo.lights.push_back(l);
		}
	}
}

} // end namespace anki
