#ifndef _PARTICLEEMITTER_H_
#define _PARTICLEEMITTER_H_

#include "Common.h"
#include "Node.h"

class ParticleEmitter: public Node
{
	public:

		class Particle: public Node
		{

		};

		Vec<Particle*> particles;
};

#endif
