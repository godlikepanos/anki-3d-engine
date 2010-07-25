#ifndef MOTIONSTATE_H
#define MOTIONSTATE_H

#include "Common.h"
#include "PhyCommon.h"
#include "SceneNode.h"


/**
 *
 */
class MotionState: public btMotionState
{
	public:
		MotionState(const btTransform& initialTransform, SceneNode& node_);

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

inline MotionState::MotionState(const btTransform& initialTransform, SceneNode& node_):
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

	/*btQuaternion rot = worldTrans.getRotation();
	node.rotationLspace = Mat3(Quat(toAnki(rot)));
	btVector3 pos = worldTrans.getOrigin();
	node.translationLspace = Vec3(toAnki(pos));*/
}




#endif
