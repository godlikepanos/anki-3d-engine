#include "ParticleEmitter.h"
#include "Renderer.h"
#include "PhyCommon.h"
#include "App.h"
#include "Scene.h"


//=====================================================================================================================================
//                                                                                                                                    =
//=====================================================================================================================================
void ParticleEmitter::Particle::render()
{
	glPushMatrix();
	R::multMatrix( transformationWspace );

	glBegin( GL_POINTS );
		glVertex3fv( &(Vec3(0.0))[0] );
	glEnd();

	glPopMatrix();
}


//=====================================================================================================================================
// init                                                                                                                               =
//=====================================================================================================================================
void ParticleEmitter::init( const char* filename )
{
	// dummy props init
	maxParticleLife = 400;
	minParticleLife = 100;
	minAngle = Euler( 0.0, toRad(-30.0), toRad(10.0) );
	maxAngle = Euler( 0.0, toRad(-30.0), toRad(10.0) );
	minParticleMass = 1.0;
	maxParticleMass = 2.0;
	maxNumOfParticles = 5;
	emittionPeriod = 1000;

	// init the rest
	btCollisionShape* colShape = new btSphereShape( 0.1 );

	particles.resize( maxNumOfParticles );
	for( int i=0; i<maxNumOfParticles; i++ )
	{
		particles[i] = new Particle;

		float mass = Util::randRange( minParticleMass, maxParticleMass );
		btVector3 localInertia;
		colShape->calculateLocalInertia( mass, localInertia );
		MotionState* mState = new MotionState( btTransform::getIdentity(), particles[i] );
		btRigidBody::btRigidBodyConstructionInfo rbInfo( mass, mState, colShape, localInertia );
		btRigidBody* body = new btRigidBody( rbInfo );
		body->setActivationState(ISLAND_SLEEPING);
		app->scene->getPhyWorld()->dynamicsWorld->addRigidBody( body, PhyWorld::CG_PARTICLE, PhyWorld::CG_MAP );
	}
}
