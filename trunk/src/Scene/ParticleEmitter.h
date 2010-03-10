#ifndef _PARTICLEEMITTER_H_
#define _PARTICLEEMITTER_H_

#include "Common.h"
#include "Node.h"

/**
 *
 */
class ParticleEmitter: public Node
{
	public:

		class Particle: public MeshNode
		{
			public:
				int life; ///< Life in ms
		};

		Vec<Particle*> particles;
};

#endif
