// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/resource/ProgramPrePreprocessor.h"
#include "anki/resource/ResourceManager.h"
#include "anki/util/Exception.h"
#include "anki/util/Functions.h"
#include "anki/util/File.h"
#include "anki/util/Array.h"
#include <iomanip>
#include <cstring>

namespace anki {

//==============================================================================
// Keep the strings in that order so that the start pragmas will match to the
// ShaderType enums
static Array<const char*, 7> commands = {{
	"#pragma anki type vert", 
	"#pragma anki type tesc",
	"#pragma anki type tese",
	"#pragma anki type geom",
	"#pragma anki type frag",
	"#pragma anki type comp",
	"#pragma anki include \""}};

const U32 MAX_DEPTH = 8;

//==============================================================================
void ProgramPrePreprocessor::printSourceLines() const
{
	U i = 1;
	auto it = m_sourceLines.getBegin();
	auto end = m_sourceLines.getEnd();
	for(; it != end; ++it)
	{
		printf("%4lu %s\n", i++, &(*it)[0]);
	}
}

//==============================================================================
Error ProgramPrePreprocessor::parseFile(const CString& filename)
{
	// Parse files recursively
	Error err = parseFileForPragmas(filename, 0);

	if(!err)
	{
		err = m_sourceLines.join(m_alloc, "\n", m_shaderSource);
	}
	
	return err;
}

//==============================================================================
Error ProgramPrePreprocessor::parseFileForPragmas(
	CString filename, U32 depth)
{
	// first check the depth
	if(depth > MAX_DEPTH)
	{
		ANKI_LOGE("The include depth is too high. "
			"Probably circular includance");
		return ErrorCode::USER_DATA;
	}

	Error err = ErrorCode::NONE;

	// load file in lines
	PPPString txt;
	PPPString::ScopeDestroyer txtd(&txt, m_alloc);

	PPPStringList lines;
	PPPStringList::ScopeDestroyer linesd(&lines, m_alloc);

	File file;
	ANKI_CHECK(file.open(filename, File::OpenFlag::READ));
	ANKI_CHECK(file.readAllText(TempResourceAllocator<char>(m_alloc), txt));
	ANKI_CHECK(lines.splitString(m_alloc, txt.toCString(), '\n'));
	if(lines.getSize() < 1)
	{
		ANKI_LOGE("File is empty: %s", &filename[0]);
		return ErrorCode::USER_DATA;
	}

	for(const PPPString& line : lines)
	{
		PtrSize npos = 0;

		if(line.find("#pragma anki") == 0)
		{
			Bool malformed = true;

			Bool found;
			ANKI_CHECK(parseType(line, found));
			if(found)
			{
				malformed = false; // All OK
			}
			else if((npos = line.find(commands[6])) == 0)
			{
				// Include

				if(line.getLength() >= std::strlen(commands[6]) + 2)
				{
					PPPString filen;
					PPPString::ScopeDestroyer filend(&filen, m_alloc);
					
					ANKI_CHECK(filen.create(
						m_alloc,
						line.begin() + std::strlen(commands[6]), 
						line.end() - 1));

					PPPString filenFixed;
					PPPString::ScopeDestroyer filenFixedd(&filenFixed, m_alloc);

					ANKI_CHECK(m_manager->fixResourceFilename(
						filen.toCString(), filenFixed));

					ANKI_CHECK(parseFileForPragmas(
						filenFixed.toCString(), depth + 1));

					malformed = false; // All OK
				}
			}

			if(malformed)
			{
				ANKI_LOGE("Malformed pragma anki: %s", &line[0]);
				return ErrorCode::USER_DATA;
			}
		}
		else
		{
			ANKI_CHECK(m_sourceLines.pushBackSprintf(m_alloc, "%s", &line[0]));
		}
	}

	// Sanity checks
	if(m_type == ShaderType::COUNT)
	{
		ANKI_LOGE("Shader is missing the type");
		return ErrorCode::USER_DATA;
	}

	return err;
}

//==============================================================================
Error ProgramPrePreprocessor::parseType(const PPPString& line, Bool& found)
{
	Error err = ErrorCode::NONE;
	U i;
	found = false;

	for(i = 0; i < static_cast<U>(ShaderType::COUNT); i++)
	{
		if(line.find(commands[i]) == 0)
		{
			found = true;
			break;
		}
	}

	if(found)
	{
		if(m_type != ShaderType::COUNT)
		{
			ANKI_LOGE("The shader type is already set. Line %s", &line[0]);
			err = ErrorCode::USER_DATA;
		}
		else
		{
			m_type = static_cast<ShaderType>(i);
		}
	}

	return err;
}

} // end namespace anki
