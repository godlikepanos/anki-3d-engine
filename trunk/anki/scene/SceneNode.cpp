#include "anki/scene/SceneNode.h"
#include "anki/scene/Scene.h"
#include "anki/util/Exception.h"
#include <algorithm>
#include <boost/lexical_cast.hpp>


namespace anki {


//==============================================================================
// Statics                                                                     =
//==============================================================================

uint SceneNode::uid = 0;


//==============================================================================
// Constructor                                                                 =
//==============================================================================
SceneNode::SceneNode(SceneNodeType type_, Scene& scene_, ulong flags_,
	SceneNode* parent_)
:	type(type_),
 	flags(flags_),
 	parent(NULL),
 	scene(scene_)
{
	++uid;

	getWorldTransform().setIdentity();
	getLocalTransform().setIdentity();

	name = boost::lexical_cast<std::string>(uid);

	enableFlag(SNF_ACTIVE);

	if(parent_ != NULL)
	{
		parent_->addChild(*this);
	}

	// This goes last
	scene.registerNode(this);
}


//==============================================================================
// Destructor                                                                  =
//==============================================================================
SceneNode::~SceneNode()
{
	scene.unregisterNode(this);

	if(parent != NULL)
	{
		parent->removeChild(*this);
	}

	if(isFlagEnabled(SNF_ON_DELETE_DELETE_CHILDREN))
	{
		BOOST_REVERSE_FOREACH(SceneNode* child, children)
		{
			delete child;
		}
	}
	else
	{
		BOOST_REVERSE_FOREACH(SceneNode* child, children)
		{
			child->parent = NULL;
		}
	}
}


//==============================================================================
// updateWorldTransform                                                        =
//==============================================================================
void SceneNode::updateWorldTransform()
{
	prevWTrf = wTrf;

	if(parent)
	{
		if(isFlagEnabled(SNF_INHERIT_PARENT_TRANSFORM))
		{
			wTrf = parent->getWorldTransform();
		}
		else
		{
			wTrf = Transform::combineTransformations(
				parent->getWorldTransform(), lTrf);
		}
	}
	else // else copy
	{
		wTrf = lTrf;
	}
}


//==============================================================================
// addChild                                                                    =
//==============================================================================
void SceneNode::addChild(SceneNode& child)
{
	ASSERT(child.parent == NULL); // Child already has parent

	child.parent = this;
	children.push_back(&child);
}


//==============================================================================
// removeChild                                                                 =
//==============================================================================
void SceneNode::removeChild(SceneNode& child)
{
	std::vector<SceneNode*>::iterator it = std::find(children.begin(),
		children.end(), &child);

	if(it == children.end())
	{
		throw ANKI_EXCEPTION("Child not found");
	}

	children.erase(it);
	child.parent = NULL;
}


//==============================================================================
// Move(s)                                                                     =
//==============================================================================

void SceneNode::moveLocalX(float distance)
{
	Vec3 x_axis = lTrf.getRotation().getColumn(0);
	getLocalTransform().getOrigin() += x_axis * distance;
}


void SceneNode::moveLocalY(float distance)
{
	Vec3 y_axis = lTrf.getRotation().getColumn(1);
	getLocalTransform().getOrigin() += y_axis * distance;
}


void SceneNode::moveLocalZ(float distance)
{
	Vec3 z_axis = lTrf.getRotation().getColumn(2);
	getLocalTransform().getOrigin() += z_axis * distance;
}


} // end namespace
