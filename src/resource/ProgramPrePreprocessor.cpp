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
	"#pragma anki include"}};

const U32 MAX_DEPTH = 8;

//==============================================================================
void ProgramPrePreprocessor::printSourceLines() const
{
	for(U32 i = 0; i < m_sourceLines.size(); ++i)
	{
		std::cout << std::setw(3) << (i + 1) << ": " 
			<< m_sourceLines[i] << std::endl;
	}
}

//==============================================================================
void ProgramPrePreprocessor::parseFile(const char* filename)
{
	try
	{
		// Parse files recursively
		parseFileForPragmas(filename);

		m_shaderSource = m_sourceLines.join("\n");
	}
	catch(Exception& e)
	{
		throw ANKI_EXCEPTION("Loading file failed: %s", filename) << e;
	}
}

//==============================================================================
void ProgramPrePreprocessor::parseFileForPragmas(
	const std::string& filename, U32 depth)
{
	// first check the depth
	if(depth > MAX_DEPTH)
	{
		throw ANKI_EXCEPTION("The include depth is too high. "
			"Probably circular includance");
	}

	// load file in lines
	String txt;
	StringList lines;
	File(filename.c_str(), File::OpenFlag::READ).readAllText(txt);
	lines = StringList::splitString(txt.c_str(), '\n');
	if(lines.size() < 1)
	{
		throw ANKI_EXCEPTION("File is empty: %s", filename.c_str());
	}

	for(const std::string& line : lines)
	{
		std::string::size_type npos = 0;
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
			std::string filen = {line, strlen(commands[6]), std::string::npos};
			trimString(filen, " \"", filen);

			filen = 
				ResourceManagerSingleton::get().fixResourcePath(filen.c_str());

			parseFileForPragmas(filen, depth + 1);
		}
		else
		{
			gotPragmaAnki = false;
			m_sourceLines.push_back(line);
		}

		// Sanity check
		if(expectPragmaAnki && !gotPragmaAnki)
		{
			throw ANKI_EXCEPTION("Malformed pragma anki: %s", line.c_str());
		}
	}

	// Sanity checks
	if(m_type == ShaderType::COUNT)
	{
		throw ANKI_EXCEPTION("Shader is missing the type");
	}
}

//==============================================================================
Bool ProgramPrePreprocessor::parseType(const std::string& line)
{
	U i;
	Bool found = false;

	for(i = 0; i < (U)ShaderType::COUNT; i++)
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
				line.c_str());
		}

		m_type = (ShaderType)i;
	}

	return found;
}

} // end namespace anki
