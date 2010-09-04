#include <algorithm>
#include "Scene.h"
#include "SkelNode.h"
#include "Camera.h"
#include "MeshNode.h"
#include "Light.h"
#include "Controller.h"
#include "Material.h"
#include "ParticleEmitter.h"


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
Scene::Scene(Object* parent):
	Object(parent)
{
	ambientCol = Vec3(0.1, 0.05, 0.05)*4;
	sunPos = Vec3(0.0, 1.0, -1.0) * 50.0;

	phyWorld = new Physics(this);
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
		case SceneNode::SNT_MESH:
			putBackNode(meshNodes, static_cast<MeshNode*>(node));
			break;
		case SceneNode::SNT_SKELETON:
			putBackNode(skelNodes, static_cast<SkelNode*>(node));
			break;
		case SceneNode::SNT_SKEL_MODEL:
			// ToDo
			break;
		case SceneNode::SNT_PARTICLE_EMITTER:
			putBackNode(particleEmitters, static_cast<ParticleEmitter*>(node));
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
		case SceneNode::SNT_MESH:
			eraseNode(meshNodes, static_cast<MeshNode*>(node));
			break;
		case SceneNode::SNT_SKELETON:
			eraseNode(skelNodes, static_cast<SkelNode*>(node));
			break;
		case SceneNode::SNT_SKEL_MODEL:
			// ToDo
			break;
		case SceneNode::SNT_PARTICLE_EMITTER:
			eraseNode(particleEmitters, static_cast<ParticleEmitter*>(node));
			break;
	};
}


//======================================================================================================================
// Register and Unregister controllers                                                                                 =
//======================================================================================================================
void Scene::registerController(Controller* controller)
{
	DEBUG_ERR(std::find(controllers.begin(), controllers.end(), controller) != controllers.end());
	controllers.push_back(controller);
}

void Scene::unregisterController(Controller* controller)
{
	Vec<Controller*>::iterator it = std::find(controllers.begin(), controllers.end(), controller);
	DEBUG_ERR(it == controllers.end());
	controllers.erase(it);
}


//======================================================================================================================
// updateAllWorldStuff                                                                                                 =
//======================================================================================================================
void Scene::updateAllWorldStuff()
{
	DEBUG_ERR(nodes.size() > 1024);
	SceneNode* queue [1024];
	uint head = 0, tail = 0;
	uint num = 0;


	// put the roots
	for(uint i=0; i<nodes.size(); i++)
		if(nodes[i]->parent == NULL)
			queue[tail++] = nodes[i]; // queue push

	// loop
	while(head != tail) // while queue not empty
	{
		SceneNode* obj = queue[head++]; // queue pop

		obj->updateWorldTransform();
		obj->updateTrf();
		obj->update();
		++num;

		for(uint i=0; i<obj->childs.size(); i++)
			queue[tail++] = obj->childs[i];
	}

	DEBUG_ERR(num != nodes.size());
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
