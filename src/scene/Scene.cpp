#include "anki/scene/Scene.h"
#include "anki/scene/Camera.h"
#include "anki/util/Exception.h"
#include "anki/scene/VisibilityTester.h"

namespace anki {

//==============================================================================
Scene::Scene()
{
	ambientCol = Vec3(0.1, 0.05, 0.05) * 2;
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
void Scene::update(float prevUpdateTime, float crntTime, Renderer& r)
{
	// First do the movable updates
	for(SceneNode* n : nodes)
	{
		Movable* m = n->getMovable();
		if(m)
		{
			m->update();
		}
	}

	// Then the rest
	for(SceneNode* n : nodes)
	{
		n->frameUpdate(prevUpdateTime, crntTime, Timestamp::getTimestamp());

		Spatial* sp = n->getSpatial();
		if(sp && sp->getSpatialTimestamp() == Timestamp::getTimestamp())
		{
			for(Sector* sector : sectors)
			{
				if(sector->placeSceneNode(n))
				{
					//ANKI_LOGI("Placing: " << n->getName());
					continue;
				}
			}
		}
	}

	doVisibilityTests(*mainCam, r);

#if 0
	for(SceneNode* n : nodes)
	{
		if(n->getSpatial()
			&& n->getSpatial()->getSpatialLastUpdateFrame() == frame)
		{
			std::cout << "Spatial updated on: " << n->getName()
				<< std::endl;
		}

		if(n->getMovable()
			&& n->getMovable()->getMovableLastUpdateFrame() == frame)
		{
			std::cout << "Movable updated on: " << n->getName()
				<< std::endl;
		}

		if(n->getFrustumable()
			&& n->getFrustumable()->getFrustumableLastUpdateFrame() == frame)
		{
			std::cout << "Frustumable updated on: " << n->getName()
				<< std::endl;
		}
	}
#endif
}

//==============================================================================
void Scene::doVisibilityTests(Camera& cam, Renderer& r)
{
	Frustumable* f = cam.getFrustumable();
	ANKI_ASSERT(f != nullptr);
	vtester.test(*f, *this, r);
}

} // end namespace anki
