// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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
	Error err = parseFileIncludes(filename, 0);

	if(!err)
	{
		m_sourceLines.join(m_alloc, "\n", m_shaderSource);
	}

	return err;
}

Error ShaderLoader::parseFileIncludes(ResourceFilename filename, U32 depth)
{
	// first check the depth
	if(depth > MAX_INCLUDE_DEPTH)
	{
		ANKI_RESOURCE_LOGE("The include depth is too high. "
						   "Probably circular includance");
		return ErrorCode::USER_DATA;
	}

	// load file in lines
	StringAuto txt(m_alloc);
	StringListAuto lines(m_alloc);

	ResourceFilePtr file;
	ANKI_CHECK(m_manager->getFilesystem().openFile(filename, file));
	ANKI_CHECK(file->readAllText(TempResourceAllocator<char>(m_alloc), txt));
	lines.splitString(txt.toCString(), '\n');
	if(lines.getSize() < 1)
	{
		ANKI_RESOURCE_LOGE("File is empty: %s", &filename[0]);
		return ErrorCode::USER_DATA;
	}

	for(const String& line : lines)
	{
		static const CString token = "#include \"";

		if(line.find(token) == 0)
		{
			// - Expect something between the quotes
			// - Expect the last char to be a quote
			if(line.getLength() >= token.getLength() + 2 && line[line.getLength() - 1] == '\"')
			{
				StringAuto filen(m_alloc);
				filen.create(line.begin() + token.getLength(), line.end() - 1);

				ANKI_CHECK(parseFileIncludes(filen.toCString(), depth + 1));
			}
			else
			{
				ANKI_RESOURCE_LOGE("Malformed #include: %s", &line[0]);
				return ErrorCode::USER_DATA;
			}
		}
		else
		{
			m_sourceLines.pushBackSprintf(m_alloc, "%s", &line[0]);
		}
	}

	return ErrorCode::NONE;
}

} // end namespace anki
