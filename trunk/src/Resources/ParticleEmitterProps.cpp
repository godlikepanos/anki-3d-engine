#include <cstring>
#include "ParticleEmitterProps.h"


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
ParticleEmitterPropsStruct::ParticleEmitterPropsStruct():
	particleLifeMargin(0.0),
	forceDirectionMargin(0.0),
	forceMagnitudeMargin(0.0),
	particleMassMargin(0.0),
	gravityMargin(0.0),
	startingPos(0.0),
	startingPosMargin(0.0),
	usingWorldGrav(true)
{}


//======================================================================================================================
// Constructor [copy]                                                                                                  =
//======================================================================================================================
ParticleEmitterPropsStruct::ParticleEmitterPropsStruct(const ParticleEmitterPropsStruct& a)
{
	memcpy(this, &(const_cast<ParticleEmitterPropsStruct&>(a)), sizeof(ParticleEmitterPropsStruct));
}


//======================================================================================================================
// load                                                                                                                =
//======================================================================================================================
bool ParticleEmitterProps::load(const char* filename)
{
	// dummy load
}
