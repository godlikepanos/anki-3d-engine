// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/StringList.h>
#include <anki/Gr.h>
#include <anki/resource/Common.h>
#include <anki/resource/Material.h>

namespace anki
{

// Forward
class XmlElement;

/// Material loader variable. It's the information on whatever is inside \<input\>
class MaterialLoaderInputVariable : public NonCopyable
{
public:
	StringAuto m_name;
	StringListAuto m_value;
	StringAuto m_line;

	ShaderVariableDataType m_type = ShaderVariableDataType::NONE;
	BuiltinMaterialVariableId m_builtin = BuiltinMaterialVariableId::NONE;

	// Flags
	class Flags
	{
	public:
		Bool8 m_inBlock = false;
		Bool8 m_texture = false;
		Bool8 m_builtin = false;
		Bool8 m_const = false;
		Bool8 m_instanced = false;
		Bool8 m_inShadow = false;
		Bool8 m_specialBuiltin = false;

		Bool operator==(const Flags& b) const
		{
			return memcmp(this, &b, sizeof(*this)) == 0;
		}
	} m_flags;

	I16 m_binding = -1; ///< Texture unit
	I16 m_index = -1;
	ShaderVariableBlockInfo m_blockInfo;

	ShaderTypeBit m_shaderDefinedMask = ShaderTypeBit::NONE; ///< Defined in
	ShaderTypeBit m_shaderReferencedMask = ShaderTypeBit::NONE; ///< Referenced

	MaterialLoaderInputVariable(GenericMemoryPoolAllocator<U8> alloc)
		: m_name(alloc)
		, m_value(alloc)
		, m_line(alloc)
	{
	}

	MaterialLoaderInputVariable(MaterialLoaderInputVariable&& b)
		: m_name(std::move(b.m_name))
		, m_value(std::move(b.m_value))
		, m_line(std::move(b.m_line))
	{
		move(b);
	}

	~MaterialLoaderInputVariable()
	{
	}

	MaterialLoaderInputVariable& operator=(MaterialLoaderInputVariable&& b)
	{
		m_name = std::move(b.m_name);
		m_value = std::move(b.m_value);
		m_line = std::move(b.m_line);
		move(b);
		return *this;
	}

	Bool duplicate(const MaterialLoaderInputVariable& b) const
	{
		return b.m_name == m_name && b.m_value == m_value && b.m_type == m_type && b.m_builtin == m_builtin
			&& b.m_flags == m_flags;
	}

	CString typeStr() const;

private:
	void move(MaterialLoaderInputVariable& b);
};

/// Creator of shader programs. This class parses between
/// @code <material></material> @endcode and creates the source of a custom program.
///
/// @note Be carefull when you change the methods. Some change may create more unique shaders and this is never good.
class MaterialLoader
{
public:
	using Input = MaterialLoaderInputVariable;

	explicit MaterialLoader(GenericMemoryPoolAllocator<U8> alloc);

	~MaterialLoader();

	ANKI_USE_RESULT Error parseXmlDocument(const XmlDocument& doc);

	/// Get the shader source code
	const String& getShaderSource(ShaderType shaderType_) const
	{
		U shaderType = enumToType(shaderType_);
		ANKI_ASSERT(!m_sourceBaked[shaderType].isEmpty());
		return m_sourceBaked[shaderType];
	}

	/// After parsing the program mutate the sources for a specific use case.
	void mutate(const RenderingKey& key);

	/// Iterate all the variables.
	template<typename TFunc>
	ANKI_USE_RESULT Error iterateAllInputVariables(TFunc func) const
	{
		for(const Input& in : m_inputs)
		{
			if(!in.m_flags.m_const && !in.m_flags.m_specialBuiltin)
			{
				ANKI_CHECK(func(in));
			}
		}
		return ErrorCode::NONE;
	}

	Bool getTessellationEnabled() const
	{
		return m_tessellation;
	}

	U32 getUniformBlockSize() const
	{
		return m_blockSize;
	}

	Bool isInstanced() const
	{
		return m_instanced;
	}

	U getLodCount() const
	{
		return m_lodCount;
	}

	Bool getShadowEnabled() const
	{
		return m_shadow;
	}

	Bool isForwardShading() const
	{
		return m_forwardShading;
	}

private:
	GenericMemoryPoolAllocator<char> m_alloc;
	Array<StringListAuto, 5> m_source; ///< Shader program final source
	Array<StringAuto, 5> m_sourceBaked; ///< Final source baked
	ListAuto<Input> m_inputs;
	ShaderTypeBit m_uniformBlockReferencedMask = ShaderTypeBit::NONE;
	U32 m_blockSize = 0;
	Bool8 m_instanced = false;
	U32 m_texBinding = 0;
	ShaderTypeBit m_instanceIdMask = ShaderTypeBit::NONE;
	Bool8 m_tessellation = false;
	U8 m_lodCount = 0;
	Bool8 m_shadow = true;
	Bool8 m_forwardShading = false;
	U32 m_nextIndex = 0;

	/// Parse what is within the
	/// @code <programs></programs> @endcode
	ANKI_USE_RESULT Error parseProgramsTag(const XmlElement& el);

	/// Parse what is within the
	/// @code <program></program> @endcode
	ANKI_USE_RESULT Error parseProgramTag(const XmlElement& el);

	/// Parse what is within the @code <inputs></inputs> @endcode
	ANKI_USE_RESULT Error parseInputsTag(const XmlElement& programEl);

	void processInputs();

	/// Parse what is within the @code <operation></operation> @endcode
	ANKI_USE_RESULT Error parseOperationTag(
		const XmlElement& el, ShaderType glshader, ShaderTypeBit glshaderbit, String& out);
};

} // end namespace anki
