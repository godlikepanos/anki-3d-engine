#include "anki/scene/VisibilityTester.h"
#include "anki/scene/Scene.h"
#include "anki/scene/Camera.h"
#include "anki/scene/Renderable.h"


namespace anki {


//==============================================================================
VisibilityTester::~VisibilityTester()
{}


//==============================================================================
void VisibilityTester::test(Frustumable& cam, Scene& scene,
	VisibilityInfo& vinfo)
{
	for(SceneNode* node : scene.getAllNodes())
	{
		Renderable* r = node->getRenderable();
		Frustumable* fr = node->getFrustumable();
		Spatial* sp = node->getSpatial();

		if(!sp || (!fr && !r))
		{
			continue;
		}

		if(!cam.insideFrustum(*sp))
		{
			continue;
		}


		if(fr)
		{
			vinfo.frustumables.push_back(fr);
		}

		if(r)
		{
			r->enableFlag(Renderable::RF_VISIBLE);
			vinfo.renderables.push_back(r);
		}
	}
}


} // end namespace
