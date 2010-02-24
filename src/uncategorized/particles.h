/*
#ifndef _PARTICLES_H_
#define _PARTICLES_H_

#include "common.h"
#include "gmath.h"
#include "app.h"
#include "object.h"

#define PARTICLE_VELS_NUM 2


===============================================================================================================================================================
particle_t                                                                                                                                                    =
===============================================================================================================================================================

class particle_t: public object_t
{
	public:
		vec3_t velocity[PARTICLE_VELS_NUM];
		vec3_t acceleration[PARTICLE_VELS_NUM];
		int life; // frames to death

		particle_t(): object_t(LIGHT) {life = 0;}

		void render();
		void renderDepth() {}
};



========
particle_emitter_t
========

class particle_emitter_t: public object_t
{
	private:
		struct velocity_t
		{
			Euler angs[2]; // MIN MAX
			float   magnitude;
			float   acceleration_magnitude;
			bool    rotatable;
			vec3_t  vel[2];
		};

	public:
		Vec<particle_t> particles;

		int start_frame;
		int stop_frame;
		int frame;
		bool repeat_emittion;
		int particle_life[2];  // []
		int particles_per_frame;

		vec3_t particles_translation_lspace[2]; // []

		// velocities
		velocity_t velocities[PARTICLE_VELS_NUM];

		//float initial_fade; // []
		//int fade_frame;
		//float fade_rate;
		//
		//float initial_size; // []
		//int size_frame;
		//float size_rate;


		particle_emitter_t(): object_t(LIGHT) {}
		~particle_emitter_t(){ particles.clear(); }

		void init();
		void ReInitParticle( particle_t& particle );
		void render();
		void renderDepth() {}
};

#endif
*/
