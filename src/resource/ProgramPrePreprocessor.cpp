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
	for(U i = 0; i < m_sourceLines.size(); ++i)
	{
		printf("%4lu %s\n", i + 1, &m_sourceLines[i][0]);
	}
}

//==============================================================================
void ProgramPrePreprocessor::parseFile(const CString& filename)
{
	try
	{
		auto alloc = m_shaderSource.getAllocator();

		// Parse files recursively
		parseFileForPragmas(
			PPPString(filename, alloc), 
			0);

		m_shaderSource = m_sourceLines.join("\n");
	}
	catch(Exception& e)
	{
		throw ANKI_EXCEPTION("Loading file failed: %s", &filename[0]) << e;
	}
}

//==============================================================================
void ProgramPrePreprocessor::parseFileForPragmas(
	const PPPString& filename, U32 depth)
{
	// first check the depth
	if(depth > MAX_DEPTH)
	{
		throw ANKI_EXCEPTION("The include depth is too high. "
			"Probably circular includance");
	}

	// load file in lines
	auto alloc = m_shaderSource.getAllocator();
	PPPString txt(alloc);
	PPPStringList lines(alloc);
	File(filename.toCString(), File::OpenFlag::READ).readAllText(txt);
	lines = PPPStringList::splitString(txt.toCString(), '\n', alloc);
	if(lines.size() < 1)
	{
		throw ANKI_EXCEPTION("File is empty: %s", &filename[0]);
	}

	for(const PPPString& line : lines)
	{
		PtrSize npos = 0;
		Bool expectPragmaAnki = false;
		Bool gotPragmaAnki = true;

		if(line.find("#pragma anki") == 0)
		{
			expectPragmaAnki = true;
		}

		if(parseType(line))
		{
			// Do nothing
		}
		else if((npos = line.find(commands[6])) == 0)
		{
			// Include

			if(line.getLength() >= std::strlen(commands[6]) + 2)
			{
				PPPString filen(
					line.begin() + std::strlen(commands[6]), 
					line.end() - 2, 
					alloc);

				filen = m_manager->fixResourceFilename(filen.toCString());

				parseFileForPragmas(filen, depth + 1);
			}
			else
			{
				gotPragmaAnki = false;
			}
		}
		else
		{
			gotPragmaAnki = false;
			m_sourceLines.push_back(line);
		}

		// Sanity check
		if(expectPragmaAnki && !gotPragmaAnki)
		{
			throw ANKI_EXCEPTION("Malformed pragma anki: %s", &line[0]);
		}
	}

	// Sanity checks
	if(m_type == ShaderType::COUNT)
	{
		throw ANKI_EXCEPTION("Shader is missing the type");
	}
}

//==============================================================================
Bool ProgramPrePreprocessor::parseType(const PPPString& line)
{
	U i;
	Bool found = false;

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
			throw ANKI_EXCEPTION("The shader type is already set. Line %s",
				&line[0]);
		}

		m_type = static_cast<ShaderType>(i);
	}

	return found;
}

} // end namespace anki
