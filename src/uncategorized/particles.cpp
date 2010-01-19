/*
#include "particles.h"
#include "renderer.h"
#include "util.h"

using namespace std;

#define MIN 0
#define MAX 1
#define VEL0 0
#define VEL1 1


=============================================================================================================================================================
Render @ particle_t                                                                                                                                         =
=============================================================================================================================================================

void particle_t::Render()
{
	if( !life ) return;
	//glPointSize( 4.0 );
	--life;

	// calc new pos
	float dt = ( float(app::timer_tick)/1000 );
	vec3_t s_vel; // sigma velocity
	s_vel = vec3_t( 0.0, 0.0, 0.0 );
	for( int i=0; i<PARTICLE_VELS_NUM; i++ )
	{
		velocity[i] = (acceleration[i] * dt) + velocity[i];
		s_vel += velocity[i];
	}

	translation_lspace = s_vel * dt + translation_lspace;

	quat_t q;
	q.CalcFrom2Vec3( vec3_t(1.0, 0.0, 0.0), s_vel );
	rotation_lspace = mat3_t( q );

	UpdateWorldTransform();


	// draw the point
	glColor3f( 1.0, 0.0, 0.0 );
	glBegin( GL_POINTS );
		glVertex3fv( &translation_wspace[0] );
	glEnd();

	// draw axis
	if( 1 )
	{
		glPushMatrix();
		r::MultMatrix( transformation_wspace );

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
				glVertex3fv( &translation_wspace[0] );
				glVertex3fv( &(velocity[i]+translation_wspace)[0] );
			glEnd();
		}

		// draw the Sv
		glColor3f( 1.0, 1.0, 1.0 );
		glBegin( GL_LINES );
			glVertex3fv( &translation_wspace[0] );
			glVertex3fv( &(s_vel+translation_wspace)[0] );
		glEnd();
	}
}



=============================================================================================================================================================
Init @ particle_emitter_t                                                                                                                                   =
=============================================================================================================================================================

void particle_emitter_t::Init()
{
	particles.resize(200);

	particle_life[0] = 200;
	particle_life[1] = 400;
	particles_per_frame = 1;

	velocities[VEL0].angs[MIN] = euler_t( 0.0, ToRad(-30.0), ToRad(10.0) );
	velocities[VEL0].angs[MAX] = euler_t( 0.0, ToRad(30.0), ToRad(45.0) );
	velocities[VEL0].magnitude = 5.0f;
	velocities[VEL0].acceleration_magnitude = -0.1f;
	velocities[VEL0].rotatable = true;

	velocities[VEL1].angs[MIN] = euler_t( 0.0, 0.0, ToRad(270.0) );
	velocities[VEL1].angs[MAX] = euler_t( 0.0, 0.0, ToRad(270.0) );
	velocities[VEL1].magnitude = 0.0f;
	velocities[VEL1].acceleration_magnitude = 1.0f;
	velocities[VEL1].rotatable = false;

	particles_translation_lspace[0] = vec3_t( 0.0, 0.0, 0.0 );
	particles_translation_lspace[1] = vec3_t( 0.0, 0.0, 0.0 );
}



==============================================================================================================================================================
ReInitParticle @ particle_emitter_t                                                                                                                          =
==============================================================================================================================================================

void particle_emitter_t::ReInitParticle( particle_t& particle )
{
	// life
	particle.life = util::RandRange( particle_life[MIN], particle_life[MAX] );

	// pos
	particle.translation_lspace = vec3_t(
		util::RandRange( particles_translation_lspace[MIN].x, particles_translation_lspace[MAX].x ),
		util::RandRange( particles_translation_lspace[MIN].y, particles_translation_lspace[MAX].y ),
		util::RandRange( particles_translation_lspace[MIN].z, particles_translation_lspace[MAX].z )
	);
	particle.rotation_lspace = mat3_t::GetIdentity();
	particle.scale_lspace = 1.0;

	particle.parent = this;
	particle.UpdateWorldTransform();

	particle.translation_lspace = particle.translation_wspace;
	particle.rotation_lspace = particle.rotation_wspace;
	particle.scale_lspace = particle.scale_wspace;
	particle.parent = NULL;

	// initial velocities
	for( int i=0; i<PARTICLE_VELS_NUM; i++ )
	{
		euler_t tmp_angs = euler_t(
			util::RandRange( velocities[i].angs[MIN].x, velocities[i].angs[MAX].x ),
			util::RandRange( velocities[i].angs[MIN].y, velocities[i].angs[MAX].y ),
			util::RandRange( velocities[i].angs[MIN].z, velocities[i].angs[MAX].z )
		);
		mat3_t m3;
		m3 = mat3_t(tmp_angs);
		if( velocities[i].rotatable ) m3 = rotation_wspace * m3;
		particle.velocity[i] = particle.acceleration[i] = m3 * vec3_t( 1.0, 0.0, 0.0 );
		particle.velocity[i] *= velocities[i].magnitude;
		particle.acceleration[i] *= velocities[i].acceleration_magnitude;

	}
}



=============================================================================================================================================================
Render @ particle_emitter_t                                                                                                                                 =
=============================================================================================================================================================

void particle_emitter_t::Render()
{
	UpdateWorldTransform();

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
		particles[i].Render();

	// render the debug cube
	if( 1 )
	{
		glEnable( GL_DEPTH_TEST );
		glDisable( GL_TEXTURE_2D );
		glDisable( GL_BLEND );
		//glLineWidth( 2.0 );
		glPolygonMode( GL_FRONT, GL_LINE );

		glPushMatrix();

		UpdateWorldTransform();
		r::MultMatrix( transformation_wspace );

		glColor3f( 0.0, 1.0, 0.0 );

		r::dbg::RenderCube();

		glPolygonMode( GL_FRONT, GL_FILL );
		glPopMatrix();
	}
}
*/
