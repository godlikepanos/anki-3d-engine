// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/ResourceManager.h>
#include <anki/util/StdTypes.h>
#include <anki/util/StringList.h>
#include <anki/Gr.h>

namespace anki
{

/// Helper class used for shader program loading. The class adds the include preprocessor directive.
class ShaderLoader
{
public:
	/// It loads a file and parses it
	/// @param[in] filename The file to load
	ShaderLoader(ResourceManager* manager)
		: m_alloc(manager->getTempAllocator())
		, m_manager(manager)
	{
	}

	~ShaderLoader()
	{
		m_shaderSource.destroy(m_alloc);
		m_sourceLines.destroy(m_alloc);
	}

	/// Run an include preprocessor in a GLSL file.
	/// @param filename The file to parse.
	ANKI_USE_RESULT Error parseFile(const ResourceFilename& filename);

	/// Run an include preprocessor in some GLSL source.
	/// @param src The source to parse.
	ANKI_USE_RESULT Error parseSourceString(CString src)
	{
		ANKI_CHECK(parseSource(src, 0));
		m_sourceLines.join(m_alloc, "\n", m_shaderSource);
		return Error::NONE;
	}

	CString getShaderSource() const
	{
		ANKI_ASSERT(!m_shaderSource.isEmpty());
		return m_shaderSource.toCString();
	}

	ShaderType getShaderType() const
	{
		ANKI_ASSERT(m_type != ShaderType::COUNT);
		return m_type;
	}

protected:
	TempResourceAllocator<U8> m_alloc;

	/// The final program source
	String m_shaderSource;

	/// The parseFileForPragmas fills this
	StringList m_sourceLines;

	/// Shader type
	ShaderType m_type = ShaderType::COUNT;

	/// Keep the manager for some path conversions.
	ResourceManager* m_manager;

	/// A recursive function that parses a file for #include and updates the output.
	/// @param filename The file to parse
	/// @param depth The #line in GLSL does not support filename so an depth it being used. It also tracks the
	///        includance depth
	ANKI_USE_RESULT Error parseFileIncludes(ResourceFilename filename, U32 depth);

	/// Parse some source text.
	ANKI_USE_RESULT Error parseSource(CString src, U depth);

	void printSourceLines() const; ///< For debugging
};

} // end namespace anki
