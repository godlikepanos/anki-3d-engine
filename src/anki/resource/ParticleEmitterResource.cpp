// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/ParticleEmitterResource.h>
#include <anki/resource/ResourceManager.h>
#include <anki/resource/ModelResource.h>
#include <anki/util/StringList.h>
#include <anki/misc/Xml.h>
#include <cstring>

namespace anki
{

static ANKI_USE_RESULT Error xmlVec3(const XmlElement& el_, const CString& str, Vec3& out)
{
	Error err = Error::NONE;
	XmlElement el;

	err = el_.getChildElementOptional(str, el);
	if(err || !el)
	{
		return err;
	}

	err = el.getVec3(out);
	return err;
}

static ANKI_USE_RESULT Error xmlF32(const XmlElement& el_, const CString& str, F32& out)
{
	Error err = Error::NONE;
	XmlElement el;

	err = el_.getChildElementOptional(str, el);
	if(err || !el)
	{
		return err;
	}

	F64 tmp;
	err = el.getNumber(tmp);
	if(!err)
	{
		out = tmp;
	}

	return err;
}

static ANKI_USE_RESULT Error xmlU32(const XmlElement& el_, const CString& str, U32& out)
{
	Error err = Error::NONE;
	XmlElement el;

	err = el_.getChildElementOptional(str, el);
	if(err || !el)
	{
		return err;
	}

	I64 tmp;
	err = el.getNumber(tmp);
	if(!err)
	{
		out = static_cast<U32>(tmp);
	}

	return err;
}

ParticleEmitterProperties& ParticleEmitterProperties::operator=(const ParticleEmitterProperties& b)
{
	std::memcpy(this, &b, sizeof(ParticleEmitterProperties));
	return *this;
}

void ParticleEmitterProperties::updateFlags()
{
	m_forceEnabled = !isZero(m_particle.m_forceDirection.getLengthSquared());
	m_forceEnabled = m_forceEnabled || !isZero(m_particle.m_forceDirectionDeviation.getLengthSquared());
	m_forceEnabled =
		m_forceEnabled && (m_particle.m_forceMagnitude != 0.0 || m_particle.m_forceMagnitudeDeviation != 0.0);

	m_wordGravityEnabled = isZero(m_particle.m_gravity.getLengthSquared());
}

ParticleEmitterResource::ParticleEmitterResource(ResourceManager* manager)
	: ResourceObject(manager)
{
}

ParticleEmitterResource::~ParticleEmitterResource()
{
}

Error ParticleEmitterResource::load(const ResourceFilename& filename, Bool async)
{
	U32 tmp;

	XmlDocument doc;
	ANKI_CHECK(openFileParseXml(filename, doc));
	XmlElement rel; // Root element
	ANKI_CHECK(doc.getChildElement("particleEmitter", rel));

	// XML load
	//
	ANKI_CHECK(xmlF32(rel, "life", m_particle.m_life));
	ANKI_CHECK(xmlF32(rel, "lifeDeviation", m_particle.m_lifeDeviation));

	ANKI_CHECK(xmlF32(rel, "mass", m_particle.m_mass));
	ANKI_CHECK(xmlF32(rel, "massDeviation", m_particle.m_massDeviation));

	ANKI_CHECK(xmlF32(rel, "size", m_particle.m_size));
	ANKI_CHECK(xmlF32(rel, "sizeDeviation", m_particle.m_sizeDeviation));
	ANKI_CHECK(xmlF32(rel, "sizeAnimation", m_particle.m_sizeAnimation));

	ANKI_CHECK(xmlF32(rel, "alpha", m_particle.m_alpha));
	ANKI_CHECK(xmlF32(rel, "alphaDeviation", m_particle.m_alphaDeviation));

	tmp = m_particle.m_alphaAnimation;
	ANKI_CHECK(xmlU32(rel, "alphaAnimationEnabled", tmp));
	m_particle.m_alphaAnimation = tmp;

	ANKI_CHECK(xmlVec3(rel, "forceDirection", m_particle.m_forceDirection));
	ANKI_CHECK(xmlVec3(rel, "forceDirectionDeviation", m_particle.m_forceDirectionDeviation));
	ANKI_CHECK(xmlF32(rel, "forceMagnitude", m_particle.m_forceMagnitude));
	ANKI_CHECK(xmlF32(rel, "forceMagnitudeDeviation", m_particle.m_forceMagnitudeDeviation));

	ANKI_CHECK(xmlVec3(rel, "gravity", m_particle.m_gravity));
	ANKI_CHECK(xmlVec3(rel, "gravityDeviation", m_particle.m_gravityDeviation));

	ANKI_CHECK(xmlVec3(rel, "startingPosition", m_particle.m_startingPos));
	ANKI_CHECK(xmlVec3(rel, "startingPositionDeviation", m_particle.m_startingPosDeviation));

	ANKI_CHECK(xmlU32(rel, "maxNumberOfParticles", m_maxNumOfParticles));

	ANKI_CHECK(xmlF32(rel, "emissionPeriod", m_emissionPeriod));
	ANKI_CHECK(xmlU32(rel, "particlesPerEmittion", m_particlesPerEmittion));
	tmp = m_usePhysicsEngine;
	ANKI_CHECK(xmlU32(rel, "usePhysicsEngine", tmp));
	m_usePhysicsEngine = tmp;

	XmlElement el;
	CString cstr;
	ANKI_CHECK(rel.getChildElement("material", el));
	ANKI_CHECK(el.getText(cstr));
	ANKI_CHECK(getManager().loadResource(cstr, m_material, async));

	// sanity checks
	//

	static const char* ERROR = "Particle emmiter: Incorrect or missing value %s";

	if(m_particle.m_life <= 0.0)
	{
		ANKI_RESOURCE_LOGE(ERROR, "life");
		return Error::USER_DATA;
	}

	if(m_particle.m_life - m_particle.m_lifeDeviation <= 0.0)
	{
		ANKI_RESOURCE_LOGE(ERROR, "lifeDeviation");
		return Error::USER_DATA;
	}

	if(m_particle.m_size <= 0.0)
	{
		ANKI_RESOURCE_LOGE(ERROR, "size");
		return Error::USER_DATA;
	}

	if(m_maxNumOfParticles < 1)
	{
		ANKI_RESOURCE_LOGE(ERROR, "maxNumOfParticles");
		return Error::USER_DATA;
	}

	if(m_emissionPeriod <= 0.0)
	{
		ANKI_RESOURCE_LOGE(ERROR, "emissionPeriod");
		return Error::USER_DATA;
	}

	if(m_particlesPerEmittion < 1)
	{
		ANKI_RESOURCE_LOGE(ERROR, "particlesPerEmission");
		return Error::USER_DATA;
	}

	// Calc some stuff
	//
	updateFlags();

	return Error::NONE;
}

void ParticleEmitterResource::getRenderingInfo(U lod, ShaderProgramPtr& prog) const
{
	lod = min<U>(lod, m_lodCount - 1);

	RenderingKey key(Pass::GB_FS, lod, 1);
	const MaterialVariant& variant = m_material->getOrCreateVariant(key);
	prog = variant.getShaderProgram();
}

} // end namespace anki
