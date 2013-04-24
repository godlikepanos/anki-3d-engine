#ifndef ANKI_RESOURCE_MATERIAL_SHADER_PROGRAM_CREATOR_H
#define ANKI_RESOURCE_MATERIAL_SHADER_PROGRAM_CREATOR_H

#include "anki/util/StringList.h"

namespace anki {

class XmlElement;

/// Creator of shader programs. This class parses between
/// <shaderProgam></shaderProgram> located inside a <material></material>
/// and creates the source of a custom program.
///
/// @note Be carefull when you change the methods. Create as less unique 
///       shaders as possible
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
		U32 foundIn = 0; ///< Found in shader
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

private:
	enum Shader
	{
		VERTEX = 1,
		FRAGMENT = 2
	}

	/// The lines of the shader program source
	StringList srcLines;

	std::string source; ///< Shader program final source

	PtrVector<Input> inputs;

	Bool enableUniformBlocks;

	/// Parse what is within the
	/// @code <shaderProgram></shaderProgram> @endcode
	void parseShaderProgramTag(const XmlElement& el);

	/// Parse what is within the
	/// @code <shader></shader> @endcode
	void parseShaderTag(const XmlElement& el);

	/// Parse what is within the @code <inputs></inputs> @endcode
	void parseInputsTag(const XmlElement& programEl);

	/// Parse what is within the @code <operation></operation> @endcode
	void parseOperationTag(const XmlElement& el, Shader shader);
};

} // end namespace anki

#endif
