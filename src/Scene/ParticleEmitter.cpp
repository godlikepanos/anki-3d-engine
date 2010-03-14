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
	if( lifeTillDeath < 0 ) return;

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
		particles[i]->body = body;
		body->setActivationState(ISLAND_SLEEPING);
		app->scene->getPhyWorld()->getDynamicsWorld()->addRigidBody( body, PhyWorld::CG_PARTICLE, PhyWorld::CG_MAP );
	}
}


//=====================================================================================================================================
// update                                                                                                                             =
//=====================================================================================================================================
void ParticleEmitter::update()
{
	uint crntTime = app->getTicks();

	// deactivate the dead particles
	for( Vec<Particle*>::iterator it=particles.begin(); it!=particles.end(); ++it )
	{
		Particle& part = **it;

		part.lifeTillDeath -= crntTime-timeOfPrevUpdate;
		if( part.lifeTillDeath < 1 )
		{
			part.body->setActivationState( ISLAND_SLEEPING );
		}
	}

	if( (crntTime - timeOfPrevEmittion) > emittionPeriod )
	{
		timeOfPrevEmittion = crntTime;

		//for(  )
	}

	timeOfPrevUpdate = crntTime;
}
