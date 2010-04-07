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
	minDirection = Vec3( -0.1, 1.0, 0.0 );
	maxDirection = Vec3( 0.1, 1.0, 0.0 );
	minForceMagnitude = 1.0;
	maxForceMagnitude = 2.0;
	minParticleMass = 1.0;
	maxParticleMass = 2.0;
	minGravity = Vec3( 0.0, 0.0, 0.0 );
	minGravity = Vec3( 0.0, -1.0, 0.0 );
	minInitialPos = Vec3( -1.0, -1.0, -1.0 );
	maxInitialPos = Vec3( 1.0, 1.0, 1.0 );
	maxNumOfParticles = 5;
	emittionPeriod = 1000;

	// init the rest
	btCollisionShape* colShape = new btSphereShape( 0.1 );

	particles.resize( maxNumOfParticles );
	for( uint i=0; i<maxNumOfParticles; i++ )
	{
		particles[i] = new Particle;

		float mass = Util::randRange( minParticleMass, maxParticleMass );
		btVector3 localInertia;
		colShape->calculateLocalInertia( mass, localInertia );
		MotionState* mState = new MotionState( btTransform::getIdentity(), particles[i] );
		btRigidBody::btRigidBodyConstructionInfo rbInfo( mass, mState, colShape, localInertia );
		btRigidBody* body = new btRigidBody( rbInfo );
		particles[i]->body = body;
		//body->setActivationState(ISLAND_SLEEPING);
		app->scene->getPhyWorld()->getDynamicsWorld()->addRigidBody( body, PhyWorld::CG_PARTICLE, PhyWorld::CG_MAP );
	}
}


//=====================================================================================================================================
// update                                                                                                                             =
//=====================================================================================================================================
void ParticleEmitter::update()
{
	uint crntTime = app->getTicks();

	// decrease particle life and deactivate the dead particles
	for( Vec<Particle*>::iterator it=particles.begin(); it!=particles.end(); ++it )
	{
		Particle* part = *it;

		part->lifeTillDeath -= crntTime-timeOfPrevUpdate;
		if( part->lifeTillDeath < 1 )
		{
			// ToDo see how bullet can deactivate bodies
		}
	}

	// emit particles
	DEBUG_ERR( particlesPerEmittion == 0 );
	if( (crntTime - timeOfPrevEmittion) > emittionPeriod )
	{
		uint partNum = 0;
		for( Vec<Particle*>::iterator it=particles.begin(); it!=particles.end(); ++it )
		{
			Particle* part = *it;
			if( part->lifeTillDeath > 0 ) continue;

			// reinit particle
			part->lifeTillDeath = Util::randRange( minParticleLife, maxParticleLife );

			Vec3 forceDir = Vec3( Util::randRange( minDirection.x , maxDirection.x ), Util::randRange( minDirection.y , maxDirection.y ),
			                                       Util::randRange( minDirection.z , maxDirection.z ) );
			forceDir.normalize();
			part->body->applyCentralForce( toBt( forceDir * Util::randRange( minForceMagnitude, maxForceMagnitude ) ) );

			minParticleMass = 1.0;
			maxParticleMass = 2.0;

			Vec3 grav = Vec3( Util::randRange(minGravity.x,maxGravity.x), Util::randRange(minGravity.y,maxGravity.y),
			                  Util::randRange(minGravity.z,maxGravity.z) );
			part->body->setGravity( toBt( grav ) );

			//part->body->get


			// do the rest
			++partNum;
			if( partNum >= particlesPerEmittion ) break;
		}

		timeOfPrevEmittion = crntTime;
	}

	timeOfPrevUpdate = crntTime;
}


//=====================================================================================================================================
// render                                                                                                                             =
//=====================================================================================================================================
void ParticleEmitter::render()
{
	float vertPositions[] = { maxInitialPos.x, maxInitialPos.y, maxInitialPos.z,   // right top front
	                          minInitialPos.x, maxInitialPos.y, maxInitialPos.z,   // left top front
	                          minInitialPos.x, minInitialPos.y, maxInitialPos.z,   // left bottom front
	                          maxInitialPos.x, minInitialPos.y, maxInitialPos.z,   // right bottom front
	                          maxInitialPos.x, maxInitialPos.y, minInitialPos.z,   // right top back
	                          minInitialPos.x, maxInitialPos.y, minInitialPos.z,   // left top back
	                          minInitialPos.x, minInitialPos.y, minInitialPos.z,   // left bottom back
	                          maxInitialPos.x, minInitialPos.y, minInitialPos.z }; // right bottom back

	uint vertIndices [] = { 0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0, 4, 1, 5, 3, 7, 2, 6 };

	glPushMatrix();
	R::multMatrix( transformationWspace );

	R::color3( Vec3(1.0) );

	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 3, GL_FLOAT, 0, vertPositions );
	glDrawElements( GL_LINES, sizeof(vertIndices)/sizeof(uint), GL_UNSIGNED_INT, vertIndices );
	glDisableClientState( GL_VERTEX_ARRAY );

	glPopMatrix();
}
