#include "ParticleEmitter.h"
#include "MainRenderer.h"
#include "PhyCommon.h"
#include "App.h"
#include "Scene.h"
#include "Util.h"


//======================================================================================================================
// render                                                                                                              =
//======================================================================================================================
void ParticleEmitter::Particle::render()
{
	/*if(lifeTillDeath < 0) return;

	glPushMatrix();
	app->getMainRenderer()->multMatrix(getWorldTransform());

	glBegin(GL_POINTS);
		glVertex3fv(&(Vec3(0.0))[0]);
	glEnd();

	glPopMatrix();*/
}


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void ParticleEmitter::init(const char* filename)
{
	particleEmitterProps.loadRsrc(filename);

	// copy the resource to me
	ParticleEmitterPropsStruct& me = *this;
	ParticleEmitterPropsStruct& other = *particleEmitterProps.get();
	me = other;
}


//======================================================================================================================
// update                                                                                                              =
//======================================================================================================================
void ParticleEmitter::update()
{
	float crntTime = SDL_GetTicks() / 1000.0;

	// deactivate the dead particles
	for(uint i=0; i<particles.size(); i++)
	{
		Particle& p = particles[i];
		if(p.timeOfDeath < 0.0) continue; // its already dead so dont deactivate it again

		if(p.timeOfDeath < crntTime)
		{
			//cout << "Killing " << i << " " << p.timeOfDeath << endl;
			p.body->setActivationState(DISABLE_SIMULATION);
			p.timeOfDeath = -1.0;
		}
	}


	if((crntTime - timeOfPrevEmittion) > emittionPeriod)
	{
		uint partNum = 0;
		for(uint i=0; i<particles.size(); i++)
		{
			Particle& p = particles[i];
			if(p.timeOfDeath > 0.0) continue; // its alive so skip it

			// life
			if(particleLifeMargin != 0.0)
				p.timeOfDeath = crntTime + particleLife + Util::randFloat(particleLifeMargin) * 2.0 - particleLifeMargin;
			else
				p.timeOfDeath = crntTime + particleLife;

			//cout << "Time of death " << p.timeOfDeath << endl;
			//cout << "Particle life " << p.timeOfDeath - crntTime << endl;

			// activate it (Bullet stuff)
			p.body->forceActivationState(ACTIVE_TAG);
			p.body->activate();
			p.body->clearForces();
			p.body->setLinearVelocity(btVector3(0.0, 0.0, 0.0));
			p.body->setAngularVelocity(btVector3(0.0, 0.0, 0.0));

			//cout << p.body->internalGetDeltaAngularVelocity() << endl;

			// force
			if(hasForce)
			{
				Vec3 forceDir;
				if(forceDirectionMargin != Vec3(0.0))
				{
					for(int i=0; i<3; i++)
					{
						forceDir[i] = forceDirection[i] + Util::randFloat(forceDirectionMargin[i]) * 2.0 - forceDirectionMargin[i];
					}
				}
				else
				{
					forceDir = forceDirection;
				}
				forceDir.normalize();
				forceDir = worldTransform.getRotation() * forceDir; // the forceDir depends on the particle emitter rotation
				Vec3 force;

				if(forceMagnitudeMargin != 0.0)
					force = forceDir * (forceMagnitude + Util::randFloat(forceMagnitudeMargin) * 2.0 - forceMagnitudeMargin);
				else
					force = forceDir * forceMagnitude;

				p.body->applyCentralForce(toBt(force));
			}

			// gravity
			if(!usingWorldGrav)
			{
				Vec3 grav;
				if(gravityMargin != Vec3(0.0, 0.0, 0.0))
				{
					for(int i=0; i<3; i++)
					{
						grav[i] = gravity[i] + Util::randFloat(gravityMargin[i]) * 2.0 - gravityMargin[i];
					}
				}
				else
				{
					grav = gravity;
				}
				p.body->setGravity(toBt(grav));
			}

			// starting pos
			Vec3 pos; // in local space
			if(startingPosMargin != Vec3(0.0))
			{
				for(int i=0; i<3; i++)
				{
					pos[i] = startingPos[i] + Util::randFloat(startingPosMargin[i]) * 2.0 - startingPosMargin[i];
				}
			}
			else
			{
				pos = startingPos;
			}
			pos.transform(worldTransform);
			btTransform trf;
			trf.setIdentity();
			trf.setOrigin(toBt(pos));
			p.body->setWorldTransform(trf);

			// do the rest
			++partNum;
			if(partNum >= particlesPerEmittion) break;
		} // end for all particles

		timeOfPrevEmittion = crntTime;
	} // end if can emit

	timeOfPrevUpdate = crntTime;
}


//======================================================================================================================
// render                                                                                                              =
//======================================================================================================================
void ParticleEmitter::render()
{
	glPolygonMode(GL_FRONT, GL_LINE);
	Renderer::Dbg::setColor(Vec4(1.0));
	Renderer::Dbg::setModelMat(Mat4(getWorldTransform()));
	Renderer::Dbg::renderCube();
	glPolygonMode(GL_FRONT, GL_FILL);
}
