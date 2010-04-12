#include "PhyWorld.h"


//=====================================================================================================================================
// createNewRigidBody                                                                                                                 =
//=====================================================================================================================================
btRigidBody* PhyWorld::createNewRigidBody( float mass, const Transform& startTransform, btCollisionShape* shape, Node* node )
{
	DEBUG_ERR( (!shape || shape->getShapeType() != INVALID_SHAPE_PROXYTYPE) )

	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	bool isDynamic = (mass != 0.0);

	btVector3 localInertia( 0, 0, 0 );
	if( isDynamic )
		shape->calculateLocalInertia( mass,localInertia );


	MotionState* myMotionState = new MotionState( toBt(startTransform), node );

	btRigidBody::btRigidBodyConstructionInfo cInfo( mass, myMotionState, shape, localInertia );

	btRigidBody* body = new btRigidBody( cInfo );
	body->setContactProcessingThreshold( defaultContactProcessingThreshold );

	dynamicsWorld->addRigidBody( body );
	return body;
}

