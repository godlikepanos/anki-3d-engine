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

/// @addtogroup resource
/// @{

/// @memberof ParticleEmitterResource2
class ParticleEmitterProperty
{
public:
	ParticleEmitterProperty() = default;

	ParticleEmitterProperty(const ParticleEmitterProperty&) = delete; // Non-copyable

	ParticleEmitterProperty& operator=(const ParticleEmitterProperty&) = delete; // Non-copyable

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

// Specialize the ParticleEmitterProperty::getValue
#define ANKI_SPECIALIZE_GET_VALUE(type, member) \
	template<> \
	inline const type& ParticleEmitterProperty::getValue<type>() const \
	{ \
		ANKI_ASSERT(m_dataType == ShaderVariableDataType::k##type); \
		return member; \
	}
#define ANKI_SVDT_MACRO(type, baseType, rowCount, columnCount, isIntagralType) ANKI_SPECIALIZE_GET_VALUE(type, ANKI_CONCATENATE(m_, type))
#include <AnKi/Gr/ShaderVariableDataType.def.h>
#undef ANKI_SPECIALIZE_GET_VALUE

/// This is the a particle emitter resource containing shader and properties.
/// XML format:
/// @code
///	<particleEmitter>
/// 	<shaderProgram name="name of the shader" />
///			[<mutation>
///				<mutator name="str" value="value"/>
///			</mutation>]
///		</shaderProgram>
///
///		[<inputs>
///			<input name="name in AnKiParticleEmitterProperties struct" value="value(s)"/>
///		</inputs>]
///	</particleEmitter>
/// @endcode
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

private:
	ResourceDynamicArray<U8> m_prefilledAnKiParticleEmitterProperties;

	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;

	Error parseShaderProgram(XmlElement shaderProgramEl, Bool async);

	Error parseMutators(XmlElement mutatorsEl, ShaderProgramResourceVariantInitInfo& variantInitInfo);

	Error parseInput(XmlElement inputEl);
};
/// @}

} // end namespace anki
