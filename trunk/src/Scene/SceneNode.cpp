#include <algorithm>
#include "SceneNode.h"
#include "Renderer.h"
#include "collision.h"
#include "Controller.h"
#include "Scene.h"
#include "App.h"


//======================================================================================================================
// commonConstructorCode                                                                                               =
//======================================================================================================================
void SceneNode::commonConstructorCode()
{
	parent = NULL;
	isCompound = false;
	bvolumeLspace = NULL;
	getWorldTransform().setIdentity();
	getLocalTransform().setIdentity();

	app->getScene().registerNode(this);
}


//======================================================================================================================
// Destructor                                                                                                          =
//======================================================================================================================
SceneNode::~SceneNode()
{
	app->getScene().unregisterNode(this);
}


//======================================================================================================================
// updateWorldTransform                                                                                                =
//======================================================================================================================
void SceneNode::updateWorldTransform()
{
	if(parent)
	{
		worldTransform = Transform::combineTransformations(parent->getWorldTransform(), localTransform);
	}
	else // else copy
	{
		worldTransform = localTransform;
	}


	// transform the bvolume
	/*if(bvolumeLspace != NULL)
	{
		DEBUG_ERR(bvolumeLspace->type!=bvolume_t::BSPHERE && bvolumeLspace->type!=bvolume_t::AABB && bvolumeLspace->type!=bvolume_t::OBB);

		switch(bvolumeLspace->type)
		{
			case bvolume_t::BSPHERE:
			{
				bsphere_t sphere = static_cast<bsphere_t*>(bvolumeLspace)->Transformed(translationWspace, rotationWspace, scaleWspace);
				*static_cast<bsphere_t*>(bvolumeLspace) = sphere;
				break;
			}

			case bvolume_t::AABB:
			{
				aabb_t aabb = static_cast<aabb_t*>(bvolumeLspace)->Transformed(translationWspace, rotationWspace, scaleWspace);
				*static_cast<aabb_t*>(bvolumeLspace) = aabb;
				break;
			}

			case bvolume_t::OBB:
			{
				obb_t obb = static_cast<obb_t*>(bvolumeLspace)->Transformed(translationWspace, rotationWspace, scaleWspace);
				*static_cast<obb_t*>(bvolumeLspace) = obb;
				break;
			}

			default:
				FATAL("What the fuck");
		}
	}*/
}


//======================================================================================================================
// Move(s)                                                                                                             =
// Move the object according to it's local axis                                                                        =
//======================================================================================================================
void SceneNode::moveLocalX(float distance)
{
	Vec3 x_axis = localTransform.rotation.getColumn(0);
	getLocalTransform().origin += x_axis * distance;
}

void SceneNode::moveLocalY(float distance)
{
	Vec3 y_axis = localTransform.rotation.getColumn(1);
	getLocalTransform().origin += y_axis * distance;
}

void SceneNode::moveLocalZ(float distance)
{
	Vec3 z_axis = localTransform.rotation.getColumn(2);
	getLocalTransform().origin += z_axis * distance;
}


//======================================================================================================================
// addChild                                                                                                            =
//======================================================================================================================
void SceneNode::addChild(SceneNode* node)
{
	if(node->parent != NULL)
	{
		ERROR("Node already has parent");
		return;
	}

	node->parent = this;
	childs.push_back(node);
}


//======================================================================================================================
// removeChild                                                                                                         =
//======================================================================================================================
void SceneNode::removeChild(SceneNode* node)
{
	Vec<SceneNode*>::iterator it = find(childs.begin(), childs.end(), node);
	if(it == childs.end())
	{
		ERROR("Child not found");
		return;
	}

	node->parent = NULL;
	childs.erase(it);
}
