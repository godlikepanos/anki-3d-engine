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
		Spatial* sp = node->getSpatial();

		if(!sp)
		{
			continue;
		}

		if(!cam.insideFrustum(*sp))
		{
			continue;
		}

		Renderable* r = node->getRenderable();
		if(r)
		{
			r->enableFlag(Renderable::RF_VISIBLE);
		}

		vinfo.nodes.push_back(node);
	}
}


} // end namespace
