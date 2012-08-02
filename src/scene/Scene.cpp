#include "anki/scene/Scene.h"
#include "anki/scene/Camera.h"
#include "anki/util/Exception.h"
#include "anki/scene/VisibilityTester.h"

namespace anki {

//==============================================================================
Scene::Scene()
{
	ambientCol = Vec3(0.1, 0.05, 0.05) * 4;
}

//==============================================================================
Scene::~Scene()
{}

//==============================================================================
void Scene::registerNode(SceneNode* node)
{
	addC(nodes, node);
	addDict(nameToNode, node);
}

//==============================================================================
void Scene::unregisterNode(SceneNode* node)
{
	removeC(nodes, node);
	removeDict(nameToNode, node);
}

//==============================================================================
void Scene::update(float prevUpdateTime, float crntTime)
{
	for(SceneNode* n : nodes)
	{
		n->frameUpdate(prevUpdateTime, crntTime, frame);
		Movable* m = n->getMovable();
		if(m)
		{
			m->update();
		}
	}

	doVisibilityTests(*mainCam);
}

//==============================================================================
void Scene::doVisibilityTests(Camera& cam)
{
	Frustumable* f = cam.getFrustumable();
	ANKI_ASSERT(f != nullptr);
	vtester.test(*f, *this);
}

} // end namespace anki
