#include <algorithm>
#include <boost/lexical_cast.hpp>
#include "SceneNode.h"
#include "Scene.h"


//======================================================================================================================
// Statics                                                                                                             =
//======================================================================================================================

uint SceneNode::uid = 0;


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
SceneNode::SceneNode(SceneNodeType type_, bool compoundFlag_, SceneNode* parent):
	Object(parent),
	type(type_),
	compoundFlag(compoundFlag_)
{
	getWorldTransform().setIdentity();
	getLocalTransform().setIdentity();
	SceneSingleton::getInstance().registerNode(this);

	name = boost::lexical_cast<std::string>(uid);

	++uid;
}


//======================================================================================================================
// Destructor                                                                                                          =
//======================================================================================================================
SceneNode::~SceneNode()
{
	SceneSingleton::getInstance().unregisterNode(this);
}


//======================================================================================================================
// updateWorldTransform                                                                                                =
//======================================================================================================================
void SceneNode::updateWorldTransform()
{
	prevWorldTransform = worldTransform;

	if(getObjParent())
	{
		const SceneNode* parent = static_cast<const SceneNode*>(getObjParent());

		if(compoundFlag)
		{
			worldTransform = parent->getWorldTransform();
		}
		else
		{
			worldTransform = Transform::combineTransformations(parent->getWorldTransform(), localTransform);
		}
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
//======================================================================================================================
void SceneNode::moveLocalX(float distance)
{
	Vec3 x_axis = localTransform.getRotation().getColumn(0);
	getLocalTransform().getOrigin() += x_axis * distance;
}

void SceneNode::moveLocalY(float distance)
{
	Vec3 y_axis = localTransform.getRotation().getColumn(1);
	getLocalTransform().getOrigin() += y_axis * distance;
}

void SceneNode::moveLocalZ(float distance)
{
	Vec3 z_axis = localTransform.getRotation().getColumn(2);
	getLocalTransform().getOrigin() += z_axis * distance;
}

