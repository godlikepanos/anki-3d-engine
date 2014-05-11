#ifndef ANKI_RESOURCE_MATERIAL_SHADER_PROGRAM_CREATOR_H
#define ANKI_RESOURCE_MATERIAL_SHADER_PROGRAM_CREATOR_H

#include "anki/util/StringList.h"
#include "anki/Gl.h"

namespace anki {

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
	class Input
	{
	public:
		std::string m_name;
		std::string m_type;
		StringList m_value;
		Bool8 m_constant;
		U16 m_arraySize;
		Bool8 m_instanced = false;

		std::string m_line;
		GLbitfield m_shaderDefinedMask = 0; ///< Defined in
		GLbitfield m_shaderReferencedMask = 0; ///< Referenced by
		Bool8 m_inBlock = true;
	};

	explicit MaterialProgramCreator(const XmlElement& pt);

	~MaterialProgramCreator();

	/// Get the shader program source code
	std::string getProgramSource(U shaderType) const
	{
		ANKI_ASSERT(m_source[shaderType].size() > 0);
		return m_source[shaderType].join("\n");
	}

	const Vector<Input>& getInputVariables() const
	{
		return m_inputs;
	}

	Bool hasTessellation() const
	{
		return m_tessellation;
	}

private:
	Array<StringList, 5> m_source; ///< Shader program final source
	Vector<Input> m_inputs;
	StringList m_uniformBlock;
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
		GLbitfield glshaderbit, std::string& out);
};

} // end namespace anki

#endif
