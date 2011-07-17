#include <algorithm>
#include <boost/lexical_cast.hpp>
#include "SceneNode.h"
#include "Scene.h"
#include "Core/Globals.h"


//==============================================================================
// Statics                                                                     =
//==============================================================================

uint SceneNode::uid = 0;


//==============================================================================
// Constructor                                                                 =
//==============================================================================
SceneNode::SceneNode(SceneNodeType type_, bool inheritParentTrfFlag_,
	SceneNode* parent)
:	Object(parent),
	type(type_),
	inheritParentTrfFlag(inheritParentTrfFlag_)
{
	getWorldTransform().setIdentity();
	getLocalTransform().setIdentity();
	SceneSingleton::getInstance().registerNode(this);

	name = boost::lexical_cast<std::string>(uid);

	++uid;

	flags = SNF_NONE;
	enableFlag(SNF_ACTIVE);
}


//==============================================================================
// Destructor                                                                  =
//==============================================================================
SceneNode::~SceneNode()
{
	SceneSingleton::getInstance().unregisterNode(this);
}


//==============================================================================
// updateWorldTransform                                                        =
//==============================================================================
void SceneNode::updateWorldTransform()
{
	prevWorldTransform = worldTransform;

	if(getObjParent())
	{
		const SceneNode* parent = static_cast<const SceneNode*>(getObjParent());

		if(inheritParentTrfFlag)
		{
			worldTransform = parent->getWorldTransform();
		}
		else
		{
			worldTransform = Transform::combineTransformations(
				parent->getWorldTransform(), localTransform);
		}
	}
	else // else copy
	{
		worldTransform = localTransform;
	}
}


//==============================================================================
// Move(s)                                                                     =
//==============================================================================

//==============================================================================
void SceneNode::moveLocalX(float distance)
{
	Vec3 x_axis = localTransform.getRotation().getColumn(0);
	getLocalTransform().getOrigin() += x_axis * distance;
}


//==============================================================================
void SceneNode::moveLocalY(float distance)
{
	Vec3 y_axis = localTransform.getRotation().getColumn(1);
	getLocalTransform().getOrigin() += y_axis * distance;
}


//==============================================================================
void SceneNode::moveLocalZ(float distance)
{
	Vec3 z_axis = localTransform.getRotation().getColumn(2);
	getLocalTransform().getOrigin() += z_axis * distance;
}

