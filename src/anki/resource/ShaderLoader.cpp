// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/ShaderLoader.h>
#include <anki/resource/ResourceManager.h>
#include <anki/resource/ResourceFilesystem.h>
#include <anki/util/Functions.h>
#include <anki/util/File.h>
#include <anki/util/Array.h>
#include <iomanip>
#include <cstring>

namespace anki
{

const U32 MAX_INCLUDE_DEPTH = 8;

void ShaderLoader::printSourceLines() const
{
	U i = 1;
	auto it = m_sourceLines.getBegin();
	auto end = m_sourceLines.getEnd();
	for(; it != end; ++it)
	{
		printf("%4d %s\n", int(i++), &(*it)[0]);
	}
}

Error ShaderLoader::parseFile(const ResourceFilename& filename)
{
	// Find the shader type
	ANKI_CHECK(fileExtensionToShaderType(filename, m_type));

	// Parse files recursively
	ANKI_CHECK(parseFileIncludes(filename, 0));

	m_sourceLines.join(m_alloc, "\n", m_shaderSource);

	return Error::NONE;
}

Error ShaderLoader::parseFileIncludes(ResourceFilename filename, U32 depth)
{
	// load file in lines
	ResourceFilePtr file;
	ANKI_CHECK(m_manager->getFilesystem().openFile(filename, file));

	StringAuto txt(m_alloc);
	ANKI_CHECK(file->readAllText(m_alloc, txt));

	ANKI_CHECK(parseSource(txt.toCString(), depth));

	return Error::NONE;
}

Error ShaderLoader::parseSource(CString src, U depth)
{
	// first check the depth
	if(depth > MAX_INCLUDE_DEPTH)
	{
		ANKI_RESOURCE_LOGE("The include depth is too high. Probably circular includance");
		return Error::USER_DATA;
	}

	// Split the file to lines
	StringListAuto lines(m_alloc);
	lines.splitString(src, '\n');
	if(lines.getSize() < 1)
	{
		ANKI_RESOURCE_LOGE("Source is empty");
		return Error::USER_DATA;
	}

	for(const String& line : lines)
	{
		// Find if we have an include
		StringAuto includeFilename(m_alloc);
		if(line.find("#") == 0)
		{
			static const CString token0 = "include \"";
			static const CString token1 = "include <";
			CString endToken = "\"";
			PtrSize pos = line.find(token0);
			if(pos == CString::NPOS)
			{
				endToken = ">";
				pos = line.find(token1);
			}

			if(pos != CString::NPOS)
			{
				const PtrSize startPos = pos + token0.getLength();
				PtrSize endPos = line.find(endToken, startPos);
				ANKI_ASSERT(endPos >= startPos);

				if(endPos == CString::NPOS || endPos - startPos < 1)
				{
					ANKI_RESOURCE_LOGE("Malformed #include: %s", line.cstr());
					return Error::USER_DATA;
				}

				includeFilename.create(line.begin() + startPos, line.begin() + endPos);
			}
		}

		// Parse new file or append line
		if(!includeFilename.isEmpty())
		{
			ANKI_CHECK(parseFileIncludes(includeFilename.toCString(), depth + 1));
		}
		else
		{
			m_sourceLines.pushBackSprintf(m_alloc, "%s", &line[0]);
		}
	}

	return Error::NONE;
}

} // end namespace anki
