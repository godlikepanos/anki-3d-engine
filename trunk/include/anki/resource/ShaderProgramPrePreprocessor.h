#ifndef ANKI_RESOURCE_SHADER_PROGRAM_PRE_PREPROCESSOR_H
#define ANKI_RESOURCE_SHADER_PROGRAM_PRE_PREPROCESSOR_H

#include "anki/util/Vector.h"
#include "anki/util/StdTypes.h"
#include "anki/util/StringList.h"
#include "anki/util/Array.h"
#include <limits>

namespace anki {

/// Shader type
enum ShaderType
{
	ST_VERTEX,
	ST_TC,
	ST_TE,
	ST_GEOMETRY,
	ST_FRAGMENT,
	ST_COMPUTE,
	ST_NUM
};

/// Helper class used for shader program loading
///
/// The class fills some of the GLSL spec deficiencies. It adds the include
/// preprocessor directive and the support to have all the shaders in the same
/// file. The file that includes all the shaders is called
/// PrePreprocessor-compatible.
///
/// The preprocessor pragmas are:
///
/// - #pragma anki start <vertexShader | tcShader | teShader |
///                       geometryShader | fragmentShader | computeShader>
/// - #pragma anki include "<filename>"
/// - #pragma anki transformFeedbackVaryings <separate|interleaved>
///   <varName> <varName>
///
/// @note The order of the "#pragma anki start" is important
class ShaderProgramPrePreprocessor
{
public:
	enum XfbBufferMode
	{
		XFBBM_NONE,
		XFBBM_INTERLEAVED,
		XFBBM_SEPARATE
	};

	/// It loads a file and parses it
	/// @param[in] filename The file to load
	/// @exception Exception
	ShaderProgramPrePreprocessor(const char* filename)
	{
		parseFile(filename);
	}

	/// Destructor does nothing
	~ShaderProgramPrePreprocessor()
	{}

	/// @name Accessors
	/// @{
	const StringList& getTranformFeedbackVaryings() const
	{
		return trffbVaryings;
	}

	const std::string& getShaderSource(ShaderType type)
	{
		if((type == ST_TC || type == ST_TE) && !enableTess)
		{
			return emptyString;
		}
		else
		{
			return shaderSources[type];
		}
	}

	XfbBufferMode getXfbBufferMode() const
	{
		return xfbBufferMode;
	}
	/// @}

protected:
	/// The pragma base class
	struct Pragma
	{
		I32 definedLine = -1;

		Bool isDefined() const
		{
			return definedLine != -1;
		}
	};

	struct CodeBeginningPragma: Pragma
	{};

	/// Names and and ids for transform feedback varyings
	StringList trffbVaryings;
	XfbBufferMode xfbBufferMode = XFBBM_NONE;
	Array<std::string, ST_NUM> shaderSources;

	/// The parseFileForPragmas fills this
	StringList sourceLines;
	Array<CodeBeginningPragma, ST_NUM> shaderStarts;

	Bool enableTess = true;

	std::string emptyString;

	/// Parse a PrePreprocessor formated GLSL file. Use the accessors to get 
	/// the output
	///
	/// @param filename The file to parse
	/// @exception Ecxeption
	void parseFile(const char* filename);

	/// Called by parseFile
	void parseFileInternal(const char* filename);

	/// A recursive function that parses a file for pragmas and updates the 
	/// output
	///
	/// @param filename The file to parse
	/// @param depth The #line in GLSL does not support filename so an
	///              depth it being used. It also tracks the includance depth
	/// @exception Exception
	void parseFileForPragmas(const std::string& filename, U32 depth = 0);

	/// Self explanatory
	void parseStartPragma(U32 shaderType, const std::string& line);

	void printSourceLines() const;  ///< For debugging
};

} // end namespace anki

#endif
