// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURCE_PROGRAM_PRE_PREPROCESSOR_H
#define ANKI_RESOURCE_PROGRAM_PRE_PREPROCESSOR_H

#include "anki/resource/ResourceManager.h"
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
private:
	using PPPStringList = StringListBase<TempResourceAllocator<char>>;
	using PPPString = TempResourceString;

public:
	/// It loads a file and parses it
	/// @param[in] filename The file to load
	ProgramPrePreprocessor(
		const CString& filename, ResourceManager* manager)
	:	m_shaderSource(manager->_getTempAllocator()),
		m_sourceLines(manager->_getTempAllocator()),
		m_manager(manager)
	{
		parseFile(filename);
	}

	~ProgramPrePreprocessor()
	{}

	/// @name Accessors
	/// @{
	const PPPString& getShaderSource() const
	{
		ANKI_ASSERT(!m_shaderSource.isEmpty());
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
	PPPString m_shaderSource;

	/// The parseFileForPragmas fills this
	PPPStringList m_sourceLines;

	/// Shader type
	ShaderType m_type = ShaderType::COUNT;

	/// Keep the manager for some path conversions.
	ResourceManager* m_manager;

	/// Parse a PrePreprocessor formated GLSL file. Use the accessors to get 
	/// the output
	///
	/// @param filename The file to parse
	void parseFile(const CString& filename);

	/// A recursive function that parses a file for pragmas and updates the 
	/// output
	///
	/// @param filename The file to parse
	/// @param depth The #line in GLSL does not support filename so an
	///              depth it being used. It also tracks the includance depth
	void parseFileForPragmas(const PPPString& filename, U32 depth);

	/// Parse the type
	Bool parseType(const PPPString& line);

	void printSourceLines() const;  ///< For debugging
};

} // end namespace anki

#endif
