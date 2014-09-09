// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/resource/ParticleEmitterResource.h"
#include "anki/resource/Model.h"
#include "anki/util/Exception.h"
#include "anki/util/StringList.h"
#include "anki/misc/Xml.h"
#include <cstring>

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

//==============================================================================
static void xmlReadVec3(const XmlElement& el_, const CString& str, Vec3& out)
{
	XmlElement el = el_.getChildElementOptional(str);

	if(!el)
	{
		return;
	}

	out = el.getVec3();
}

//==============================================================================
static void xmlReadFloat(const XmlElement& el_, const CString& str, F32& out)
{
	XmlElement el = el_.getChildElementOptional(str);

	if(!el)
	{
		return;
	}

	out = el.getFloat();
}

//==============================================================================
static void xmlReadU(const XmlElement& el_, const CString& str, U32& out)
{
	XmlElement el = el_.getChildElementOptional(str);

	if(!el)
	{
		return;
	}

	out = static_cast<U32>(el.getInt());
}

//==============================================================================
// ParticleEmitterProperties                                                   =
//==============================================================================

//==============================================================================
ParticleEmitterProperties& ParticleEmitterProperties::operator=(
	const ParticleEmitterProperties& b)
{
	std::memcpy(this, &b, sizeof(ParticleEmitterProperties));
	return *this;
}

//==============================================================================
void ParticleEmitterProperties::updateFlags()
{
	m_forceEnabled = !isZero(m_particle.m_forceDirection.getLengthSquared());
	m_forceEnabled = m_forceEnabled
		|| !isZero(m_particle.m_forceDirectionDeviation.getLengthSquared());
	m_forceEnabled = m_forceEnabled
		&& (m_particle.m_forceMagnitude != 0.0
		|| m_particle.m_forceMagnitudeDeviation != 0.0);

	m_wordGravityEnabled = isZero(m_particle.m_gravity.getLengthSquared());
}

//==============================================================================
// ParticleEmitterResource                                                     =
//==============================================================================

//==============================================================================
ParticleEmitterResource::ParticleEmitterResource()
{}

//==============================================================================
ParticleEmitterResource::~ParticleEmitterResource()
{}

//==============================================================================
void ParticleEmitterResource::load(const CString& filename, 
	ResourceInitializer& init)
{
	try
	{
		XmlDocument doc;
		doc.loadFile(filename, init.m_tempAlloc);
		loadInternal(doc.getChildElement("particleEmitter"), init);
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Failed to load particles") << e;
	}
}

//==============================================================================
void ParticleEmitterResource::loadInternal(const XmlElement& rootel,
	ResourceInitializer& init)
{
	// XML load
	//
	xmlReadFloat(rootel, "life", m_particle.m_life);
	xmlReadFloat(rootel, "lifeDeviation", m_particle.m_lifeDeviation);
	
	xmlReadFloat(rootel, "mass", m_particle.m_mass);
	xmlReadFloat(rootel, "massDeviation", m_particle.m_massDeviation);

	xmlReadFloat(rootel, "size", m_particle.m_size);
	xmlReadFloat(rootel, "sizeDeviation", m_particle.m_sizeDeviation);
	xmlReadFloat(rootel, "sizeAnimation", m_particle.m_sizeAnimation);

	xmlReadFloat(rootel, "alpha", m_particle.m_alpha);
	xmlReadFloat(rootel, "alphaDeviation", m_particle.m_alphaDeviation);
	U32 tmp = m_particle.m_alphaAnimation;
	xmlReadU(rootel, "alphaAnimationEnabled", tmp);
	m_particle.m_alphaAnimation = tmp;

	xmlReadVec3(rootel, "forceDirection", m_particle.m_forceDirection);
	xmlReadVec3(rootel, "forceDirectionDeviation", 
		m_particle.m_forceDirectionDeviation);
	xmlReadFloat(rootel, "forceMagnitude", m_particle.m_forceMagnitude);
	xmlReadFloat(rootel, "forceMagnitudeDeviation", 
		m_particle.m_forceMagnitudeDeviation);

	xmlReadVec3(rootel, "gravity", m_particle.m_gravity);
	xmlReadVec3(rootel, "gravityDeviation", m_particle.m_gravityDeviation);

	xmlReadVec3(rootel, "startingPosition", m_particle.m_startingPos);
	xmlReadVec3(rootel, "startingPositionDeviation", 
		m_particle.m_startingPosDeviation);

	xmlReadU(rootel, "maxNumberOfParticles", m_maxNumOfParticles);

	xmlReadFloat(rootel, "emissionPeriod", m_emissionPeriod);
	xmlReadU(rootel, "particlesPerEmittion", m_particlesPerEmittion);
	tmp = m_usePhysicsEngine;
	xmlReadU(rootel, "usePhysicsEngine", tmp);
	m_usePhysicsEngine = tmp;

	XmlElement el = rootel.getChildElement("material");
	m_material.load(el.getText(), &init.m_resources);

	// sanity checks
	//

	static const char* ERROR = "Particle emmiter: "
		"Incorrect or missing value %s";

	if(m_particle.m_life <= 0.0)
	{
		throw ANKI_EXCEPTION(ERROR, "life");
	}

	if(m_particle.m_life - m_particle.m_lifeDeviation <= 0.0)
	{
		throw ANKI_EXCEPTION(ERROR, "lifeDeviation");
	}

	if(m_particle.m_size <= 0.0)
	{
		throw ANKI_EXCEPTION(ERROR, "size");
	}

	if(m_maxNumOfParticles < 1)
	{
		throw ANKI_EXCEPTION(ERROR, "maxNumOfParticles");
	}

	if(m_emissionPeriod <= 0.0)
	{
		throw ANKI_EXCEPTION(ERROR, "emissionPeriod");
	}

	if(m_particlesPerEmittion < 1)
	{
		throw ANKI_EXCEPTION(ERROR, "particlesPerEmission");
	}

	// Calc some stuff
	//
	updateFlags();
}

} // end namespace anki
