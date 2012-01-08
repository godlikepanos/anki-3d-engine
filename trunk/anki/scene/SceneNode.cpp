#include "anki/scene/SceneNode.h"
#include "anki/scene/Scene.h"


namespace anki {


//==============================================================================
SceneNode::SceneNode(const char* name_, long flags_,
	SceneNode* parent, Scene* scene_)
	: Base(this, parent), name(name_), flags(flags_), scene(scene_)
{
	scene->registerNode(this);
}


//==============================================================================
SceneNode::~SceneNode()
{
	scene->unregisterNode(this);
}


//==============================================================================
void SceneNode::updateWorldTransform()
{
	prevWTrf = wTrf;

	if(getParent())
	{
		if(isFlagEnabled(SNF_IGNORE_LOCAL_TRANSFORM))
		{
			wTrf = getParent()->getWorldTransform();
		}
		else
		{
			wTrf = Transform::combineTransformations(
				getParent()->getWorldTransform(), lTrf);
		}
	}
	else // else copy
	{
		wTrf = lTrf;
	}
}


} // end namespace
