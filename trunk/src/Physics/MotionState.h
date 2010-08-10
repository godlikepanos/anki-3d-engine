#ifndef MOTIONSTATE_H
#define MOTIONSTATE_H

#include "Common.h"
#include "PhyCommon.h"
#include "SceneNode.h"
#include "Object.h"


/**
 * A custom motion state
 */
class MotionState: public btMotionState, public Object
{
	public:
		MotionState(const btTransform& initialTransform, SceneNode& node_, Object* parent = NULL);

		~MotionState() {}

		void getWorldTransform(btTransform& worldTrans) const;

		const btTransform& getWorldTransform() const;

		void setWorldTransform(const btTransform& worldTrans);

	private:
		btTransform worldTransform;
		SceneNode& node;
};


//======================================================================================================================
// Inlines                                                                                                             =
//======================================================================================================================

inline MotionState::MotionState(const btTransform& initialTransform, SceneNode& node_, Object* parent):
	Object(parent),
	worldTransform(initialTransform),
	node(node_)
{}


inline void MotionState::getWorldTransform(btTransform& worldTrans) const
{
	worldTrans = worldTransform;
}


inline const btTransform& MotionState::getWorldTransform() const
{
	return worldTransform;
}


inline void MotionState::setWorldTransform(const btTransform& worldTrans)
{
	float originalScale = node.getLocalTransform().getScale();
	worldTransform = worldTrans;

	node.setLocalTransform(Transform(toAnki(worldTrans)));
	node.getLocalTransform().setScale(originalScale);
}




#endif
