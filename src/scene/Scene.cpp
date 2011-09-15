#include <boost/foreach.hpp>
#include <algorithm>
#include "util/Exception.h"
#include "Scene.h"
#include "VisibilityTester.h"

#include "Camera.h"
#include "Light.h"
#include "Controller.h"
#include "rsrc/Material.h"
#include "ParticleEmitterNode.h"
#include "ModelNode.h"
#include "SkinNode.h"


//==============================================================================
// Constructors & destructor                                                   =
//==============================================================================

Scene::Scene()
{
	ambientCol = Vec3(0.1, 0.05, 0.05) * 4;
	//sunPos = Vec3(0.0, 1.0, -1.0) * 50.0;

	physPhysWorld.reset(new PhysWorld);
	visibilityTester.reset(new VisibilityTester(*this));
}


Scene::~Scene()
{}


//==============================================================================
// registerNode                                                                =
//==============================================================================
void Scene::registerNode(SceneNode* node)
{
	putBackNode(nodes, node);
	
	switch(node->getSceneNodeType())
	{
		case SceneNode::SNT_LIGHT:
			putBackNode(lights, static_cast<Light*>(node));
			break;
		case SceneNode::SNT_CAMERA:
			putBackNode(cams, static_cast<Camera*>(node));
			break;
		case SceneNode::SNT_PARTICLE_EMITTER_NODE:
			putBackNode(particleEmitterNodes,
				static_cast<ParticleEmitterNode*>(node));
			break;
		case SceneNode::SNT_MODEL_NODE:
			putBackNode(modelNodes, static_cast<ModelNode*>(node));
			break;
		case SceneNode::SNT_SKIN_NODE:
			putBackNode(skinNodes, static_cast<SkinNode*>(node));
			break;
		case SceneNode::SNT_RENDERABLE_NODE:
		case SceneNode::SNT_GHOST_NODE:
			break;
	};
}


//==============================================================================
// unregisterNode                                                              =
//==============================================================================
void Scene::unregisterNode(SceneNode* node)
{
	eraseNode(nodes, node);
	
	switch(node->getSceneNodeType())
	{
		case SceneNode::SNT_LIGHT:
			eraseNode(lights, static_cast<Light*>(node));
			break;
		case SceneNode::SNT_CAMERA:
			eraseNode(cams, static_cast<Camera*>(node));
			break;
		case SceneNode::SNT_PARTICLE_EMITTER_NODE:
			eraseNode(particleEmitterNodes,
				static_cast<ParticleEmitterNode*>(node));
			break;
		case SceneNode::SNT_MODEL_NODE:
			eraseNode(modelNodes, static_cast<ModelNode*>(node));
			break;
		case SceneNode::SNT_SKIN_NODE:
			eraseNode(skinNodes, static_cast<SkinNode*>(node));
			break;
		case SceneNode::SNT_RENDERABLE_NODE:
		case SceneNode::SNT_GHOST_NODE:
			break;
	};
}


//==============================================================================
// Register and Unregister controllers                                         =
//==============================================================================
void Scene::registerController(Controller* controller)
{
	ASSERT(std::find(controllers.begin(), controllers.end(), controller) ==
		controllers.end());
	controllers.push_back(controller);
}

void Scene::unregisterController(Controller* controller)
{
	Vec<Controller*>::iterator it = std::find(controllers.begin(),
		controllers.end(), controller);
	ASSERT(it != controllers.end());
	controllers.erase(it);
}


//==============================================================================
// updateAllWorldStuff                                                         =
//==============================================================================
void Scene::updateAllWorldStuff(float prevUpdateTime, float crntTime)
{
	ASSERT(nodes.size() <= 1024);
	boost::array<SceneNode*, 1024> queue;
	uint head = 0, tail = 0;
	uint num = 0;


	// put the roots
	BOOST_FOREACH(SceneNode* node, nodes)
	{
		if(node->getParent() == NULL)
		{
			queue[tail++] = node; // queue push
		}
	}

	// loop
	while(head != tail) // while queue not empty
	{
		SceneNode* pnode = queue[head++]; // queue pop

		pnode->updateWorldTransform();
		pnode->frameUpdate(prevUpdateTime, crntTime);
		pnode->moveUpdate();
		++num;

		BOOST_FOREACH(SceneNode* node, pnode->getChildren())
		{
			queue[tail++] = node;
		}
	}

	ASSERT(num == nodes.size());
}


//==============================================================================
// updateAllControllers                                                        =
//==============================================================================
void Scene::updateAllControllers()
{
	BOOST_FOREACH(Controller* ctl, controllers)
	{
		ctl->update(0.0);
	}
}
