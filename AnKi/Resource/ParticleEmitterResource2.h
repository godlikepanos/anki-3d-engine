// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Resource/ResourceObject.h>
#include <AnKi/Math.h>

namespace anki {

class XmlElement;
class ShaderProgramResourceVariantInitInfo;

class ParticleEmitterResourceProperty
{
public:
	ParticleEmitterResourceProperty(const ParticleEmitterResourceProperty&) = delete; // Non-copyable

	ParticleEmitterResourceProperty& operator=(const ParticleEmitterResourceProperty&) = delete; // Non-copyable

	CString getName() const
	{
		return m_name;
	}

	template<typename T>
	const T& getValue() const;

private:
	ResourceString m_name;
	U32 m_offsetInAnKiParticleEmitterProperties = kMaxU32;
	ShaderVariableDataType m_dataType = ShaderVariableDataType::kNone;

	union
	{
#define ANKI_SVDT_MACRO(type, baseType, rowCount, columnCount, isIntagralType) type ANKI_CONCATENATE(m_, type);
#include <AnKi/Gr/ShaderVariableDataType.def.h>
	};
};

// Specialize the ParticleEmitterResourceProperty::getValue
#define ANKI_SPECIALIZE_GET_VALUE(type, member) \
	template<> \
	inline const type& ParticleEmitterResourceProperty::getValue<type>() const \
	{ \
		ANKI_ASSERT(m_dataType == ShaderVariableDataType::k##type); \
		return member; \
	}
#define ANKI_SVDT_MACRO(type, baseType, rowCount, columnCount, isIntagralType) ANKI_SPECIALIZE_GET_VALUE(type, ANKI_CONCATENATE(m_, type))
#include <AnKi/Gr/ShaderVariableDataType.def.h>
#undef ANKI_SPECIALIZE_GET_VALUE

// Common properties for each emitter. The rest of the properties are up to the user
class ParticleEmitterResourceCommonProperties
{
public:
	U32 m_particleCount = 0;
	F32 m_emissionPeriod = 0.0;
	U32 m_particlesPerEmission = 0;
};

// This is the a particle emitter resource containing shader and properties.
// XML format:
//	<particleEmitter>
//		<shaderProgram name="name of the shader" />
//			[<mutation>
//				<mutator name="str" value="value"/>
//			</mutation>]
//		</shaderProgram>
//
//		<!-- Common properties -->
//		<particleCount value="value" />
//		<emissionPeriod value="value" />
//		<particlesPerEmission value="value" />
//
//		[<inputs>
//			<input name="name in AnKiParticleEmitterProperties struct" value="value(s)"/>
//		</inputs>]
// </particleEmitter>
class ParticleEmitterResource2 : public ResourceObject
{
public:
	ParticleEmitterResource2(CString fname, U32 uuid)
		: ResourceObject(fname, uuid)
	{
	}

	~ParticleEmitterResource2() = default;

	/// Load it
	Error load(const ResourceFilename& filename, Bool async);

	const ParticleEmitterResourceCommonProperties& getCommonProperties() const
	{
		return m_commonProps;
	}

	ConstWeakArray<U8> getPrefilledAnKiParticleEmitterProperties() const
	{
		return m_prefilledAnKiParticleEmitterProperties;
	}

private:
	ResourceDynamicArray<U8> m_prefilledAnKiParticleEmitterProperties;

	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;

	ParticleEmitterResourceCommonProperties m_commonProps;

	Error parseShaderProgram(XmlElement shaderProgramEl, Bool async);

	Error parseMutators(XmlElement mutatorsEl, ShaderProgramResourceVariantInitInfo& variantInitInfo);

	Error parseInput(XmlElement inputEl);
};

} // end namespace anki
