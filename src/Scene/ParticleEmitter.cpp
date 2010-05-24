#include "ParticleEmitter.h"
#include "Renderer.h"
#include "PhyCommon.h"
#include "App.h"
#include "Scene.h"


//=====================================================================================================================================
// render                                                                                                                             =
//=====================================================================================================================================
void ParticleEmitter::Particle::render()
{
	if( lifeTillDeath < 0 ) return;

	glPushMatrix();
	R::multMatrix( getWorldTransform() );

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
	minStartingPos = Vec3( -1.0, -1.0, -1.0 );
	maxStartingPos = Vec3( 1.0, 1.0, 1.0 );
	maxNumOfParticles = 5;
	emittionPeriod = 1000;

	// init the particles
	btCollisionShape* colShape = new btSphereShape( 0.1 );

	particles.resize( maxNumOfParticles );
	for( uint i=0; i<maxNumOfParticles; i++ )
	{
		particles[i] = new Particle;
		float mass = Util::randRange( minParticleMass, maxParticleMass );
		btRigidBody* body = app->getScene()->getPhyWorld()->createNewRigidBody( mass, Transform::getIdentity(), colShape, particles[i],
		                                                                        PhyWorld::CG_PARTICLE, PhyWorld::CG_MAP );
		//body->forceActivationState( DISABLE_SIMULATION );
	}

	/*btDiscreteDynamicsWorld* btWorld = app->getScene()->getPhyWorld()->getDynamicsWorld();
	btWorld->getBroadphase()->resetPool( btWorld->getDispatcher() );
	btWorld->getConstraintSolver()->reset();*/
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
		if( part->lifeTillDeath < 0 ) continue; // its already dead so dont deactivate it again

		part->lifeTillDeath -= crntTime-timeOfPrevUpdate;
		if( part->lifeTillDeath < 1 )
		{
			part->body->setActivationState( DISABLE_SIMULATION );
		}
	}

	// emit new particles
	DEBUG_ERR( particlesPerEmittion == 0 );
	if( (crntTime - timeOfPrevEmittion) > emittionPeriod )
	{
		uint partNum = 0;
		for( Vec<Particle*>::iterator it=particles.begin(); it!=particles.end(); ++it )
		{
			Particle* part = *it;
			if( part->lifeTillDeath > 0 ) continue; // its alive so skip it

			// reinit a dead particle
			//

			// activate it (Bullet stuff)
			part->body->forceActivationState( ACTIVE_TAG );
			part->body->clearForces();

			// life
			if( minParticleLife != maxParticleLife )
				part->lifeTillDeath = Util::randRange( minParticleLife, maxParticleLife );
			else
				part->lifeTillDeath = minParticleLife;

			// force
			Vec3 forceDir;
			if( minDirection != maxDirection )
			{
				forceDir = Vec3( Util::randRange( minDirection.x , maxDirection.x ), Util::randRange( minDirection.y , maxDirection.y ),
			                                    Util::randRange( minDirection.z , maxDirection.z ) );
			}
			else
			{
				forceDir = minDirection;
			}
			forceDir.normalize();

			if( minForceMagnitude != maxForceMagnitude )
				part->body->applyCentralForce( toBt( forceDir * Util::randRange( minForceMagnitude, maxForceMagnitude ) ) );
			else
				part->body->applyCentralForce( toBt( forceDir * minForceMagnitude ) );

			// gravity
			Vec3 grav;
			if( minGravity != maxGravity )
			{
				grav = Vec3( Util::randRange(minGravity.x,maxGravity.x), Util::randRange(minGravity.y,maxGravity.y),
			               Util::randRange(minGravity.z,maxGravity.z) );
			}
			else
			{
				grav = minGravity;
			}
			part->body->setGravity( toBt( grav ) );

			// starting pos
			Vec3 pos;
			if( minStartingPos != maxStartingPos )
			{
				pos = Vec3( Util::randRange(minStartingPos.x,maxStartingPos.x), Util::randRange(minStartingPos.y,maxStartingPos.y),
			              Util::randRange(minStartingPos.z,maxStartingPos.z) );
			}
			else
			{
				pos = minStartingPos;
			}
			pos += getWorldTransform().getOrigin();
			part->body->setWorldTransform( toBt( Mat4( pos, Mat3::getIdentity(), 1.0 ) ) );

			// do the rest
			++partNum;
			if( partNum >= particlesPerEmittion ) break;
		} // end for all particles

		timeOfPrevEmittion = crntTime;
	} // end if can emit

	timeOfPrevUpdate = crntTime;
}


//=====================================================================================================================================
// render                                                                                                                             =
//=====================================================================================================================================
void ParticleEmitter::render()
{
	glPushMatrix();
	R::multMatrix( getWorldTransform() );

	R::color3( Vec3(1.0) );

	/*glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 3, GL_FLOAT, 0, vertPositions );
	glDrawElements( GL_LINES, sizeof(vertIndices)/sizeof(uint), GL_UNSIGNED_INT, vertIndices );
	glDisableClientState( GL_VERTEX_ARRAY );*/
	glPolygonMode( GL_FRONT, GL_LINE );
	R::Dbg::renderCube();
	glPolygonMode( GL_FRONT, GL_FILL );

	glPopMatrix();
}
