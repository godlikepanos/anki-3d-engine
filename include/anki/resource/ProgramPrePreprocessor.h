// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURCE_PROGRAM_PRE_PREPROCESSOR_H
#define ANKI_RESOURCE_PROGRAM_PRE_PREPROCESSOR_H

#include "anki/resource/ResourceManager.h"
#include "anki/util/StdTypes.h"
#include "anki/util/StringList.h"
#include "anki/Gr.h"

namespace anki {

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
	ProgramPrePreprocessor(ResourceManager* manager)
	:	m_alloc(manager->_getTempAllocator()),
		m_manager(manager)
	{}

	~ProgramPrePreprocessor()
	{
		m_shaderSource.destroy(m_alloc);
		m_sourceLines.destroy(m_alloc);
	}

	/// Parse a PrePreprocessor formated GLSL file. Use the accessors to get 
	/// the output
	///
	/// @param filename The file to parse
	ANKI_USE_RESULT Error parseFile(const CString& filename);

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

protected:
	TempResourceAllocator<U8> m_alloc;

	/// The final program source
	PPPString m_shaderSource;

	/// The parseFileForPragmas fills this
	PPPStringList m_sourceLines;

	/// Shader type
	ShaderType m_type = ShaderType::COUNT;

	/// Keep the manager for some path conversions.
	ResourceManager* m_manager;

	/// A recursive function that parses a file for pragmas and updates the 
	/// output
	///
	/// @param filename The file to parse
	/// @param depth The #line in GLSL does not support filename so an
	///              depth it being used. It also tracks the includance depth
	ANKI_USE_RESULT Error parseFileForPragmas(
		CString filename, U32 depth);

	/// Parse the type
	ANKI_USE_RESULT Error parseType(const PPPString& line, Bool& found);

	void printSourceLines() const;  ///< For debugging
};

} // end namespace anki

#endif
