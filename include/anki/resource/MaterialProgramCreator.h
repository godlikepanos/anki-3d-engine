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
	using MPStringList = BasicStringList<TempResourceAllocator<char>>; 

	class Input
	{
	public:
		Input(TempResourceAllocator<char>& alloc)
		:	m_name(alloc),
			m_type(alloc),
			m_value(alloc),
			m_line(alloc)
		{}

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
	};

	explicit MaterialProgramCreator(const XmlElement& pt, 
		TempResourceAllocator<U8>& alloc);

	~MaterialProgramCreator();

	/// Get the shader program source code
	MPString getProgramSource(U shaderType) const
	{
		ANKI_ASSERT(m_source[shaderType].size() > 0);
		return m_source[shaderType].join(CString("\n"));
	}

	const TempResourceVector<Input>& getInputVariables() const
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
	TempResourceVector<Input> m_inputs;
	MPStringList m_uniformBlock;
	GLbitfield m_uniformBlockReferencedMask = 0;
	Bool8 m_instanced = false;
	U32 m_texBinding = 0;
	GLbitfield m_instanceIdMask = 0;
	Bool8 m_tessellation = false;

	/// Parse what is within the
	/// @code <programs></programs> @endcode
	void parseProgramsTag(const XmlElement& el);

	/// Parse what is within the
	/// @code <program></program> @endcode
	void parseProgramTag(const XmlElement& el);

	/// Parse what is within the @code <inputs></inputs> @endcode
	void parseInputsTag(const XmlElement& programEl);

	/// Parse what is within the @code <operation></operation> @endcode
	void parseOperationTag(const XmlElement& el, GLenum glshader, 
		GLbitfield glshaderbit, MPString& out);
};

} // end namespace anki

#endif
