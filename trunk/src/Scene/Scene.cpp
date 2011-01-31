#include <algorithm>
#include "Exception.h"
#include "Scene.h"
#include "Camera.h"
#include "Light.h"
#include "Controller.h"
#include "Material.h"
#include "ParticleEmitter.h"
#include "ModelNode.h"


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
Scene::Scene()
{
	ambientCol = Vec3(0.1, 0.05, 0.05)*4;
	//sunPos = Vec3(0.0, 1.0, -1.0) * 50.0;

	physics.reset(new Physics);
}


//======================================================================================================================
// registerNode                                                                                                        =
//======================================================================================================================
void Scene::registerNode(SceneNode* node)
{
	putBackNode(nodes, node);
	
	switch(node->type)
	{
		case SceneNode::SNT_LIGHT:
			putBackNode(lights, static_cast<Light*>(node));
			break;
		case SceneNode::SNT_CAMERA:
			putBackNode(cameras, static_cast<Camera*>(node));
			break;
		case SceneNode::SNT_PARTICLE_EMITTER:
			putBackNode(particleEmitters, static_cast<ParticleEmitter*>(node));
			break;
		case SceneNode::SNT_MODEL:
			putBackNode(modelNodes, static_cast<ModelNode*>(node));
			break;
		case SceneNode::SNT_RENDERABLE:
			putBackNode(renderableNodes, static_cast<RenderableNode*>(node));
			break;
	};
}


//======================================================================================================================
// unregisterNode                                                                                                      =
//======================================================================================================================
void Scene::unregisterNode(SceneNode* node)
{
	eraseNode(nodes, node);
	
	switch(node->type)
	{
		case SceneNode::SNT_LIGHT:
			eraseNode(lights, static_cast<Light*>(node));
			break;
		case SceneNode::SNT_CAMERA:
			eraseNode(cameras, static_cast<Camera*>(node));
			break;
		case SceneNode::SNT_PARTICLE_EMITTER:
			eraseNode(particleEmitters, static_cast<ParticleEmitter*>(node));
			break;
		case SceneNode::SNT_MODEL:
			eraseNode(modelNodes, static_cast<ModelNode*>(node));
			break;
		case SceneNode::SNT_RENDERABLE:
			eraseNode(renderableNodes, static_cast<RenderableNode*>(node));
			break;
	};
}


//======================================================================================================================
// Register and Unregister controllers                                                                                 =
//======================================================================================================================
void Scene::registerController(Controller* controller)
{
	RASSERT_THROW_EXCEPTION(std::find(controllers.begin(), controllers.end(), controller) != controllers.end());
	controllers.push_back(controller);
}

void Scene::unregisterController(Controller* controller)
{
	Vec<Controller*>::iterator it = std::find(controllers.begin(), controllers.end(), controller);
	RASSERT_THROW_EXCEPTION(it == controllers.end());
	controllers.erase(it);
}


//======================================================================================================================
// updateAllWorldStuff                                                                                                 =
//======================================================================================================================
void Scene::updateAllWorldStuff()
{
	RASSERT_THROW_EXCEPTION(nodes.size() > 1024);
	boost::array<SceneNode*, 1024> queue;
	uint head = 0, tail = 0;
	uint num = 0;


	// put the roots
	for(uint i=0; i<nodes.size(); i++)
	{
		if(nodes[i]->getObjParent() == NULL)
		{
			queue[tail++] = nodes[i]; // queue push
		}
	}

	// loop
	while(head != tail) // while queue not empty
	{
		SceneNode* obj = queue[head++]; // queue pop

		obj->updateWorldTransform();
		obj->updateTrf();
		obj->update();
		++num;

		Object::Container::iterator it = obj->getObjChildren().begin();
		for(; it != obj->getObjChildren().end(); it++)
		{
			SceneNode* node = static_cast<SceneNode*>(*it);
			queue[tail++] = node;
		}
	}

	RASSERT_THROW_EXCEPTION(num != nodes.size());
}


//======================================================================================================================
// updateAllControllers                                                                                                =
//======================================================================================================================
void Scene::updateAllControllers()
{
	for(Vec<Controller*>::iterator it=controllers.begin(); it!=controllers.end(); it++)
	{
		(*it)->update(0.0);
	}
}
