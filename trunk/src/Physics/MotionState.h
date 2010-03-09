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
		Node* node;

	public:
		MotionState( const Mat4& transform, Node* node_ ):
			node( node_ )
		{
			DEBUG_ERR( node_==NULL );
			setWorldTransform( toBt(transform) );
		}

		virtual ~MotionState()
		{}

		virtual void getWorldTransform( btTransform &worldTrans ) const
		{
			worldTrans = toBt( node->transformationWspace );
		}

		btTransform getWorldTransform() const
		{
			return toBt( node->transformationWspace );
		}

		virtual void setWorldTransform( const btTransform& worldTrans )
		{
			node->transformationWspace = toAnki( worldTrans );
		}
};

#endif
