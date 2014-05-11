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
#define PE_EXCEPTION(x) ANKI_EXCEPTION("Particle emmiter: " \
	"Incorrect or missing value %s", x)

//==============================================================================
static void xmlReadVec3(const XmlElement& el_, const char* str, Vec3& out)
{
	XmlElement el = el_.getChildElementOptional(str);

	if(!el)
	{
		return;
	}

	StringList list;

	list = StringList::splitString(el.getText(), ' ');
	if(list.size() != 3)
	{
		throw ANKI_EXCEPTION("Expecting 3 floats for Vec3");
	}

	for(U i = 0; i < 3; i++)
	{
		out[i] = std::stof(list[i]);
	}
}

//==============================================================================
static void xmlReadFloat(const XmlElement& el_, const char* str, F32& out)
{
	XmlElement el = el_.getChildElementOptional(str);

	if(!el)
	{
		return;
	}

	out = std::stof(el.getText());
}

//==============================================================================
static void xmlReadU(const XmlElement& el_, const char* str, U32& out)
{
	XmlElement el = el_.getChildElementOptional(str);

	if(!el)
	{
		return;
	}

	out = (U32)std::stoi(el.getText());
}

//==============================================================================
// ParticleEmitterProperties                                                   =
//==============================================================================

//==============================================================================
ParticleEmitterProperties& ParticleEmitterProperties::operator=(
	const ParticleEmitterProperties& b)
{
	memcpy(this, &b, sizeof(ParticleEmitterProperties));
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
void ParticleEmitterResource::load(const char* filename)
{
	try
	{
		XmlDocument doc;
		doc.loadFile(filename);
		loadInternal(doc.getChildElement("particleEmitter"));
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Failed to load particles") << e;
	}
}

//==============================================================================
void ParticleEmitterResource::loadInternal(const XmlElement& rootel)
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
	m_material.load(el.getText());

	// sanity checks
	//

	if(m_particle.m_life <= 0.0)
	{
		throw PE_EXCEPTION("life");
	}

	if(m_particle.m_life - m_particle.m_lifeDeviation <= 0.0)
	{
		throw PE_EXCEPTION("lifeDeviation");
	}

	if(m_particle.m_size <= 0.0)
	{
		throw PE_EXCEPTION("size");
	}

	if(m_maxNumOfParticles < 1)
	{
		throw PE_EXCEPTION("maxNumOfParticles");
	}

	if(m_emissionPeriod <= 0.0)
	{
		throw PE_EXCEPTION("emissionPeriod");
	}

	if(m_particlesPerEmittion < 1)
	{
		throw PE_EXCEPTION("particlesPerEmission");
	}

	// Calc some stuff
	//
	updateFlags();
}

} // end namespace anki
