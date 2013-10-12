#ifndef ANKI_RESOURCE_MATERIAL_SHADER_PROGRAM_CREATOR_H
#define ANKI_RESOURCE_MATERIAL_SHADER_PROGRAM_CREATOR_H

#include "anki/util/StringList.h"

namespace anki {

class XmlElement;

/// Creator of shader programs. This class parses between
/// <shaderProgam></shaderProgram> located inside a <material></material>
/// and creates the source of a custom program.
///
/// @note Be carefull when you change the methods. Some change may create more
////      unique shaders and this is never good.
class MaterialShaderProgramCreator
{
public:
	struct Input
	{
		std::string name;
		std::string type;
		StringList value;
		Bool constant;
		U32 arraySize;
		std::string line;
		U32 shaders = 0; ///< Shader mask
		Bool putInBlock = false;
		Bool instanced = false;
	};

	explicit MaterialShaderProgramCreator(const XmlElement& pt, 
		Bool enableUniformBlocks = false);
	~MaterialShaderProgramCreator();

	/// Get the shader program source code. This is the one and only public
	/// member
	const std::string& getShaderProgramSource() const
	{
		return source;
	}

	const PtrVector<Input>& getInputVariables() const
	{
		return inputs;
	}

	/// At least one variable is instanced
	Bool usesInstancing() const
	{
		return instanced;
	}

	/// It has tessellation shaders
	Bool hasTessellation() const
	{
		return tessellation;
	}

	/// The fragment shader needs the vInstanceId
	Bool usesInstanceIdInFragmentShader() const
	{
		return instanceIdInFragmentShader;
	}

private:
	/// The lines of the shader program source
	StringList srcLines;

	std::string source; ///< Shader program final source

	PtrVector<Input> inputs;

	Bool enableUniformBlocks;

	Bool instanced = false;

	Bool tessellation = false;

	Bool instanceIdInFragmentShader = false;

	/// Parse what is within the
	/// @code <shaderProgram></shaderProgram> @endcode
	void parseShaderProgramTag(const XmlElement& el);

	/// Parse what is within the
	/// @code <shader></shader> @endcode
	void parseShaderTag(const XmlElement& el);

	/// Parse what is within the @code <inputs></inputs> @endcode
	void parseInputsTag(const XmlElement& programEl);

	/// Parse what is within the @code <operation></operation> @endcode
	void parseOperationTag(const XmlElement& el, U32 shader, std::string& out);
};

} // end namespace anki

#endif
