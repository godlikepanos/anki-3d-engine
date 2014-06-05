// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURCE_PROGRAM_PRE_PREPROCESSOR_H
#define ANKI_RESOURCE_PROGRAM_PRE_PREPROCESSOR_H

#include "anki/util/StdTypes.h"
#include "anki/util/StringList.h"

namespace anki {

/// Shader type
enum class ShaderType
{
	VERTEX,
	TC,
	TE,
	GEOMETRY,
	FRAGMENT,
	COMPUTE,
	COUNT
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
/// - #pragma anki type <vert | tesc | tese | geom | frag | comp>
/// - #pragma anki include "<filename>"
class ProgramPrePreprocessor
{
public:
	/// It loads a file and parses it
	/// @param[in] filename The file to load
	ProgramPrePreprocessor(const char* filename)
	{
		parseFile(filename);
	}

	~ProgramPrePreprocessor()
	{}

	/// @name Accessors
	/// @{
	const String& getShaderSource()
	{
		ANKI_ASSERT(!m_shaderSource.empty());
		return m_shaderSource;
	}

	ShaderType getShaderType() const
	{
		ANKI_ASSERT(m_type != ShaderType::COUNT);
		return m_type;
	}
	/// @}

protected:
	/// The final program source
	String m_shaderSource;

	/// The parseFileForPragmas fills this
	StringList m_sourceLines;

	/// Shader type
	ShaderType m_type = ShaderType::COUNT;

	/// Parse a PrePreprocessor formated GLSL file. Use the accessors to get 
	/// the output
	///
	/// @param filename The file to parse
	void parseFile(const char* filename);

	/// A recursive function that parses a file for pragmas and updates the 
	/// output
	///
	/// @param filename The file to parse
	/// @param depth The #line in GLSL does not support filename so an
	///              depth it being used. It also tracks the includance depth
	void parseFileForPragmas(const String& filename, U32 depth = 0);

	/// Parse the type
	Bool parseType(const String& line);

	void printSourceLines() const;  ///< For debugging
};

} // end namespace anki

#endif
