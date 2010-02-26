/*
#include "particles.h"
#include "renderer.h"
#include "Util.h"

using namespace std;

#define MIN 0
#define MAX 1
#define VEL0 0
#define VEL1 1


=============================================================================================================================================================
render @ particle_t                                                                                                                                         =
=============================================================================================================================================================

void particle_t::render()
{
	if( !life ) return;
	//glPointSize( 4.0 );
	--life;

	// calc new pos
	float dt = ( float(App::timerTick)/1000 );
	Vec3 s_vel; // sigma velocity
	s_vel = Vec3( 0.0, 0.0, 0.0 );
	for( int i=0; i<PARTICLE_VELS_NUM; i++ )
	{
		velocity[i] = (acceleration[i] * dt) + velocity[i];
		s_vel += velocity[i];
	}

	translationLspace = s_vel * dt + translationLspace;

	Quat q;
	q.setFrom2Vec3( Vec3(1.0, 0.0, 0.0), s_vel );
	rotationLspace = Mat3( q );

	updateWorldTransform();


	// draw the point
	glColor3f( 1.0, 0.0, 0.0 );
	glBegin( GL_POINTS );
		glVertex3fv( &translationWspace[0] );
	glEnd();

	// draw axis
	if( 1 )
	{
		glPushMatrix();
		r::multMatrix( transformationWspace );

		glBegin( GL_LINES );
			// x-axis
			glColor3f( 1.0, 0.0, 0.0 );
			glVertex3f( 0.0, 0.0, 0.0 );
			glVertex3f( 1.0, 0.0, 0.0 );
			// y-axis
			glColor3f( 0.0, 1.0, 0.0 );
			glVertex3f( 0.0, 0.0, 0.0 );
			glVertex3f( 0.0, 1.0, 0.0 );
			// z-axis
			glColor3f( 0.0, 0.0, 1.0 );
			glVertex3f( 0.0, 0.0, 0.0 );
			glVertex3f( 0.0, 0.0, 1.0 );
		glEnd();

		glPopMatrix();
	}

	// draw the velocities
	if( 0 )
	{
		// draw the velocities
		for( int i=0; i<PARTICLE_VELS_NUM; i++ )
		{
			glColor3f( 0.0, 0.0, 1.0 );
			glBegin( GL_LINES );
				glVertex3fv( &translationWspace[0] );
				glVertex3fv( &(velocity[i]+translationWspace)[0] );
			glEnd();
		}

		// draw the Sv
		glColor3f( 1.0, 1.0, 1.0 );
		glBegin( GL_LINES );
			glVertex3fv( &translationWspace[0] );
			glVertex3fv( &(s_vel+translationWspace)[0] );
		glEnd();
	}
}



=============================================================================================================================================================
init @ particle_emitter_t                                                                                                                                   =
=============================================================================================================================================================

void particle_emitter_t::init()
{
	particles.resize(200);

	particle_life[0] = 200;
	particle_life[1] = 400;
	particles_per_frame = 1;

	velocities[VEL0].angs[MIN] = Euler( 0.0, toRad(-30.0), toRad(10.0) );
	velocities[VEL0].angs[MAX] = Euler( 0.0, toRad(30.0), toRad(45.0) );
	velocities[VEL0].magnitude = 5.0f;
	velocities[VEL0].acceleration_magnitude = -0.1f;
	velocities[VEL0].rotatable = true;

	velocities[VEL1].angs[MIN] = Euler( 0.0, 0.0, toRad(270.0) );
	velocities[VEL1].angs[MAX] = Euler( 0.0, 0.0, toRad(270.0) );
	velocities[VEL1].magnitude = 0.0f;
	velocities[VEL1].acceleration_magnitude = 1.0f;
	velocities[VEL1].rotatable = false;

	particles_translation_lspace[0] = Vec3( 0.0, 0.0, 0.0 );
	particles_translation_lspace[1] = Vec3( 0.0, 0.0, 0.0 );
}



==============================================================================================================================================================
ReInitParticle @ particle_emitter_t                                                                                                                          =
==============================================================================================================================================================

void particle_emitter_t::ReInitParticle( particle_t& particle )
{
	// life
	particle.life = Util::randRange( particle_life[MIN], particle_life[MAX] );

	// pos
	particle.translationLspace = Vec3(
		Util::randRange( particles_translation_lspace[MIN].x, particles_translation_lspace[MAX].x ),
		Util::randRange( particles_translation_lspace[MIN].y, particles_translation_lspace[MAX].y ),
		Util::randRange( particles_translation_lspace[MIN].z, particles_translation_lspace[MAX].z )
	);
	particle.rotationLspace = Mat3::getIdentity();
	particle.scaleLspace = 1.0;

	particle.parent = this;
	particle.updateWorldTransform();

	particle.translationLspace = particle.translationWspace;
	particle.rotationLspace = particle.rotationWspace;
	particle.scaleLspace = particle.scaleWspace;
	particle.parent = NULL;

	// initial velocities
	for( int i=0; i<PARTICLE_VELS_NUM; i++ )
	{
		Euler tmp_angs = Euler(
			Util::randRange( velocities[i].angs[MIN].x, velocities[i].angs[MAX].x ),
			Util::randRange( velocities[i].angs[MIN].y, velocities[i].angs[MAX].y ),
			Util::randRange( velocities[i].angs[MIN].z, velocities[i].angs[MAX].z )
		);
		Mat3 m3;
		m3 = Mat3(tmp_angs);
		if( velocities[i].rotatable ) m3 = rotationWspace * m3;
		particle.velocity[i] = particle.acceleration[i] = m3 * Vec3( 1.0, 0.0, 0.0 );
		particle.velocity[i] *= velocities[i].magnitude;
		particle.acceleration[i] *= velocities[i].acceleration_magnitude;

	}
}



=============================================================================================================================================================
render @ particle_emitter_t                                                                                                                                 =
=============================================================================================================================================================

void particle_emitter_t::render()
{
	updateWorldTransform();

	// emitt new particles
	int remain = particles_per_frame;
	for( uint i=0; i<particles.size(); i++ )
	{
		if( particles[i].life != 0 ) continue;

		ReInitParticle( particles[i] );

		--remain;
		if( remain == 0 ) break;
	}

	// render all particles
	for( uint i=0; i<particles.size(); i++ )
		particles[i].render();

	// render the debug cube
	if( 1 )
	{
		glEnable( GL_DEPTH_TEST );
		glDisable( GL_TEXTURE_2D );
		glDisable( GL_BLEND );
		//glLineWidth( 2.0 );
		glPolygonMode( GL_FRONT, GL_LINE );

		glPushMatrix();

		updateWorldTransform();
		r::multMatrix( transformationWspace );

		glColor3f( 0.0, 1.0, 0.0 );

		r::dbg::renderCube();

		glPolygonMode( GL_FRONT, GL_FILL );
		glPopMatrix();
	}
}
*/
