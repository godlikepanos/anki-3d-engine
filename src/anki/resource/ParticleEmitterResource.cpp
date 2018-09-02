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

template<typename T>
static ANKI_USE_RESULT Error getXmlVal(const XmlElement& el, const CString& tag, T& out, Bool& found)
{
	return el.getAttributeNumberOptional(tag, out, found);
}

template<>
ANKI_USE_RESULT Error getXmlVal(const XmlElement& el, const CString& tag, Vec3& out, Bool& found)
{
	return el.getAttributeVectorOptional(tag, out, found);
}

ParticleEmitterProperties& ParticleEmitterProperties::operator=(const ParticleEmitterProperties& b)
{
	std::memcpy(this, &b, sizeof(ParticleEmitterProperties));
	return *this;
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
	XmlDocument doc;
	ANKI_CHECK(openFileParseXml(filename, doc));
	XmlElement rootEl; // Root element
	ANKI_CHECK(doc.getChildElement("particleEmitter", rootEl));

#define ANKI_XML(varName, VarName) \
	ANKI_CHECK( \
		readVar(rootEl, #varName, m_particle.m_min##VarName, m_particle.m_max##VarName, &m_particle.m_min##VarName))

	ANKI_XML(life, Life);
	ANKI_XML(mass, Mass);
	ANKI_XML(initialSize, InitialSize);
	ANKI_XML(finalSize, FinalSize);
	ANKI_XML(initialAlpha, InitialAlpha);
	ANKI_XML(finalAlpha, FinalAlpha);
	ANKI_XML(forceDirection, ForceDirection);
	ANKI_XML(forceMagnitude, ForceMagnitude);
	ANKI_XML(gravity, Gravity);
	ANKI_XML(startingPosition, StartingPosition);

#undef ANKI_XML

	XmlElement el;
	ANKI_CHECK(rootEl.getChildElement("maxNumberOfParticles", el));
	ANKI_CHECK(el.getAttributeNumber("value", m_maxNumOfParticles));

	ANKI_CHECK(rootEl.getChildElement("emissionPeriod", el));
	ANKI_CHECK(el.getAttributeNumber("value", m_emissionPeriod));

	ANKI_CHECK(rootEl.getChildElement("particlesPerEmission", el));
	ANKI_CHECK(el.getAttributeNumber("value", m_particlesPerEmission));

	ANKI_CHECK(rootEl.getChildElementOptional("usePhysicsEngine", el));
	if(el)
	{
		ANKI_CHECK(el.getAttributeNumber("value", m_usePhysicsEngine));
	}

	CString cstr;
	ANKI_CHECK(rootEl.getChildElement("material", el));
	ANKI_CHECK(el.getAttributeText("value", cstr));
	ANKI_CHECK(getManager().loadResource(cstr, m_material, async));

	return Error::NONE;
}

template<typename T>
Error ParticleEmitterResource::readVar(
	const XmlElement& rootEl, CString varName, T& minVal, T& maxVal, const T* defaultVal)
{
	XmlElement el;

	// <varName>
	ANKI_CHECK(rootEl.getChildElementOptional(varName, el));
	if(!el && !defaultVal)
	{
		ANKI_RESOURCE_LOGE("<%s> is missing", varName.cstr());
		return Error::USER_DATA;
	}

	if(!el)
	{
		maxVal = minVal = *defaultVal;
		return Error::NONE;
	}

	// value tag
	Bool found;
	ANKI_CHECK(getXmlVal(el, "value", minVal, found));
	if(found)
	{
		maxVal = minVal;
		return Error::NONE;
	}

	// min & max value tags
	ANKI_CHECK(getXmlVal(el, "min", minVal, found));
	if(!found)
	{
		ANKI_RESOURCE_LOGE("tag min is missing for <%s>", varName.cstr());
		return Error::USER_DATA;
	}

	ANKI_CHECK(getXmlVal(el, "max", maxVal, found));
	if(!found)
	{
		ANKI_RESOURCE_LOGE("tag max is missing for <%s>", varName.cstr());
		return Error::USER_DATA;
	}

	if(minVal > maxVal)
	{
		ANKI_RESOURCE_LOGE("min tag should have less value than max for <%s>", varName.cstr());
		return Error::USER_DATA;
	}

	return Error::NONE;
}

void ParticleEmitterResource::getRenderingInfo(U lod, ShaderProgramPtr& prog) const
{
	lod = min<U>(lod, m_lodCount - 1);

	RenderingKey key(Pass::FS, lod, 1, false, false);
	const MaterialVariant& variant = m_material->getOrCreateVariant(key);
	prog = variant.getShaderProgram();
}

} // end namespace anki
