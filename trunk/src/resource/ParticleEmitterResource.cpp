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
	forceEnabled = !isZero(particle.forceDirection.getLengthSquared());
	forceEnabled = forceEnabled
		|| !isZero(particle.forceDirectionDeviation.getLengthSquared());
	forceEnabled = forceEnabled
		&& (particle.forceMagnitude != 0.0
		|| particle.forceMagnitudeDeviation != 0.0);

	wordGravityEnabled = isZero(particle.gravity.getLengthSquared());
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
	xmlReadFloat(rootel, "life", particle.life);
	xmlReadFloat(rootel, "lifeDeviation", particle.lifeDeviation);
	
	xmlReadFloat(rootel, "mass", particle.mass);
	xmlReadFloat(rootel, "massDeviation", particle.massDeviation);

	xmlReadFloat(rootel, "size", particle.size);
	xmlReadFloat(rootel, "sizeDeviation", particle.sizeDeviation);
	xmlReadFloat(rootel, "sizeAnimation", particle.sizeAnimation);

	xmlReadFloat(rootel, "alpha", particle.alpha);
	xmlReadFloat(rootel, "alphaDeviation", particle.alphaDeviation);
	U32 tmp;
	xmlReadU(rootel, "alphaAnimationEnabled", tmp);
	particle.alphaAnimation = tmp;

	xmlReadVec3(rootel, "forceDirection", particle.forceDirection);
	xmlReadVec3(rootel, "forceDirectionDeviation", 
		particle.forceDirectionDeviation);
	xmlReadFloat(rootel, "forceMagnitude", particle.forceMagnitude);
	xmlReadFloat(rootel, "forceMagnitudeDeviation", 
		particle.forceMagnitudeDeviation);

	xmlReadVec3(rootel, "gravity", particle.gravity);
	xmlReadVec3(rootel, "gravityDeviation", particle.gravityDeviation);

	xmlReadVec3(rootel, "startingPosition", particle.startingPos);
	xmlReadVec3(rootel, "startingPositionDeviation", 
		particle.startingPosDeviation);

	xmlReadU(rootel, "maxNumberOfParticles", maxNumOfParticles);

	xmlReadFloat(rootel, "emissionPeriod", emissionPeriod);
	xmlReadU(rootel, "particlesPerEmittion", particlesPerEmittion);
	U32 u = usePhysicsEngine;
	xmlReadU(rootel, "usePhysicsEngine", u);
	usePhysicsEngine = u;

	XmlElement el = rootel.getChildElement("material");
	material.load(el.getText());

	// sanity checks
	//

	if(particle.life <= 0.0)
	{
		throw PE_EXCEPTION("life");
	}

	if(particle.life - particle.lifeDeviation <= 0.0)
	{
		throw PE_EXCEPTION("lifeDeviation");
	}

	if(particle.size <= 0.0)
	{
		throw PE_EXCEPTION("size");
	}

	if(maxNumOfParticles < 1)
	{
		throw PE_EXCEPTION("maxNumOfParticles");
	}

	if(emissionPeriod <= 0.0)
	{
		throw PE_EXCEPTION("emissionPeriod");
	}

	if(particlesPerEmittion < 1)
	{
		throw PE_EXCEPTION("particlesPerEmission");
	}

	// Calc some stuff
	//
	updateFlags();
}

} // end namespace anki
