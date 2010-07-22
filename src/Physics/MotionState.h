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
	protected:
		btTransform worldTransform;
		SceneNode* node;

	public:
		MotionState(const btTransform& initialTransform, SceneNode* node_):
			worldTransform(initialTransform),
			node(node_)
		{
			DEBUG_ERR(node_==NULL);
		}

		virtual ~MotionState()
		{}

		virtual void getWorldTransform(btTransform& worldTrans) const
		{
			worldTrans = worldTransform;
		}

		const btTransform& getWorldTransform() const
		{
			return worldTransform;
		}

		virtual void setWorldTransform(const btTransform& worldTrans)
		{
			float originalScale = node->getLocalTransform().getScale();
			worldTransform = worldTrans;

			node->setLocalTransform(Transform(toAnki(worldTrans)));
			node->getLocalTransform().setScale(originalScale);

			/*btQuaternion rot = worldTrans.getRotation();
			node->rotationLspace = Mat3(Quat(toAnki(rot)));
			btVector3 pos = worldTrans.getOrigin();
			node->translationLspace = Vec3(toAnki(pos));*/
		}
};




#endif
