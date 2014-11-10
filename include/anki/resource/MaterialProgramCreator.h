// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURCE_MATERIAL_SHADER_PROGRAM_CREATOR_H
#define ANKI_RESOURCE_MATERIAL_SHADER_PROGRAM_CREATOR_H

#include "anki/util/StringList.h"
#include "anki/Gl.h"
#include "anki/resource/Common.h"

namespace anki {

// Forward
class XmlElement;

/// Creator of shader programs. This class parses between
/// @code <shaderProgams></shaderPrograms> @endcode located inside a 
/// @code <material></material> @endcode and creates the source of a custom 
/// program.
///
/// @note Be carefull when you change the methods. Some change may create more
///       unique shaders and this is never good.
class MaterialProgramCreator
{
public:
	using MPString = TempResourceString; 
	using MPStringList = StringListBase<TempResourceAllocator<char>>;

	class Input: public NonCopyable
	{
	public:
		TempResourceAllocator<U8> m_alloc;
		MPString m_name;
		MPString m_type;
		MPStringList m_value;
		Bool8 m_constant;
		U16 m_arraySize;
		Bool8 m_instanced = false;

		MPString m_line;
		GLbitfield m_shaderDefinedMask = 0; ///< Defined in
		GLbitfield m_shaderReferencedMask = 0; ///< Referenced by
		Bool8 m_inBlock = true;

		Input()
		{}

		Input(Input&& b)
		{
			move(b);
		}

		~Input()
		{
			m_name.destroy(m_alloc);
			m_type.destroy(m_alloc);
			m_value.destroy(m_alloc);
			m_line.destroy(m_alloc);
		}

		Input& operator=(Input&& b)
		{
			move(b);
			return *this;
		}

		void move(Input& b)
		{
			m_alloc = std::move(b.m_alloc);
			m_name = std::move(b.m_name);
			m_type = std::move(b.m_type);
			m_value = std::move(b.m_value);
			m_constant = b.m_constant;
			m_arraySize = b.m_arraySize;
			m_line = std::move(b.m_line);
			m_shaderDefinedMask = b.m_shaderDefinedMask;
			m_shaderReferencedMask = b.m_shaderReferencedMask;
			m_inBlock = b.m_inBlock;
		}
	};

	explicit MaterialProgramCreator(TempResourceAllocator<U8>& alloc);

	~MaterialProgramCreator();

	/// Parse what is within the
	/// @code <programs></programs> @endcode
	ANKI_USE_RESULT Error parseProgramsTag(const XmlElement& el);

	/// Get the shader program source code
	const MPString& getProgramSource(ShaderType shaderType_) const
	{
		U shaderType = enumToType(shaderType_);
		ANKI_ASSERT(!m_sourceBaked[shaderType].isEmpty());
		return m_sourceBaked[shaderType];
	}

	const List<Input, TempResourceAllocator<U8>>& getInputVariables() const
	{
		return m_inputs;
	}

	Bool hasTessellation() const
	{
		return m_tessellation;
	}

private:
	TempResourceAllocator<char> m_alloc; 
	Array<MPStringList, 5> m_source; ///< Shader program final source
	Array<MPString, 5> m_sourceBaked; ///< Final source baked
	List<Input, TempResourceAllocator<U8>> m_inputs;
	MPStringList m_uniformBlock;
	GLbitfield m_uniformBlockReferencedMask = 0;
	Bool8 m_instanced = false;
	U32 m_texBinding = 0;
	GLbitfield m_instanceIdMask = 0;
	Bool8 m_tessellation = false;

	/// Parse what is within the
	/// @code <program></program> @endcode
	ANKI_USE_RESULT Error parseProgramTag(const XmlElement& el);

	/// Parse what is within the @code <inputs></inputs> @endcode
	ANKI_USE_RESULT Error parseInputsTag(const XmlElement& programEl);

	/// Parse what is within the @code <operation></operation> @endcode
	ANKI_USE_RESULT Error parseOperationTag(
		const XmlElement& el, GLenum glshader, 
		GLbitfield glshaderbit, MPString& out);
};

} // end namespace anki

#endif
