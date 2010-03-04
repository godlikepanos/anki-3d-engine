#ifndef _MOTIONSTATE_H_
#define _MOTIONSTATE_H_

#include "Common.h"
#include "PhyCommon.h"
#include "Node.h"


/**
 *
 */
class MotionState: public btMotionState
{
	protected:
		btTransform mPos1;
		Node* node;

	public:
		MotionState( const btTransform& initialPos, Node* node_ ):
			mPos1( initialPos ),
			node( node_ )
		{
			DEBUG_ERR( node_==NULL );
		}

		virtual ~MotionState()
		{}

		virtual void getWorldTransform( btTransform &worldTrans ) const
		{
			worldTrans = mPos1;
		}

		virtual void setWorldTransform( const btTransform &worldTrans )
		{
			btQuaternion rot = worldTrans.getRotation();
			node->rotationLspace = Mat3( Quat( toAnki(rot) ) );
			btVector3 pos = worldTrans.getOrigin();
			node->translationLspace = Vec3( toAnki(pos) );
		}
};

#endif
