#include "anki/scene/Scene.h"
#include "anki/util/Exception.h"
#include "anki/scene/VisibilityTester.h"


namespace anki {


//==============================================================================
Scene::Scene()
{
	ambientCol = Vec3(0.1, 0.05, 0.05) * 4;

	//physPhysWorld.reset(new PhysWorld);
	//visibilityTester.reset(new VisibilityTester(*this));
}


//==============================================================================
Scene::~Scene()
{}


//==============================================================================
void Scene::registerNode(SceneNode* node)
{
	if(nodeExists(node->getName().c_str()))
	{
		throw ANKI_EXCEPTION("Scene node already exists: " + node.getName());
	}

	nodes.push_back(node);
	nameToNode[node->getName().c_str()] = node;
}


//==============================================================================
void Scene::unregisterNode(SceneNode* node)
{
	Types<SceneNode>::Iterator it;
	for(it : nodes)
	{
		if(*it == node)
		{
			break;
		}
	}
}


} // end namespace
