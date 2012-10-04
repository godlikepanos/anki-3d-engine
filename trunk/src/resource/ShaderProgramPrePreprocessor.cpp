#include "anki/resource/ShaderProgramPrePreprocessor.h"
#include "anki/misc/Parser.h"
#include "anki/util/Filesystem.h"
#include "anki/util/Exception.h"
#include "anki/util/Functions.h"
#include <iomanip>
#include <cstring>
#include <iostream>

namespace anki {

//==============================================================================

// Keep the strings in that order so that the start pragmas will match to the
// ShaderType enums
static Array<const char*, 7> commands = {{
	"#pragma anki start vertexShader",
	"#pragma anki start teShader",
	"#pragma anki start tcShader",
	"#pragma anki start geometryShader",
	"#pragma anki start fragmentShader",
	"#pragma anki include",
	"#pragma anki transformFeedbackVaryings"}};

static_assert(ST_VERTEX == 0 && ST_FRAGMENT == 4, "See file");

static const char* ENTRYPOINT_NOT_DEFINED = "Entry point not defined: ";

const U32 MAX_DEPTH = 8;

//==============================================================================
void ShaderProgramPrePreprocessor::printSourceLines() const
{
	for(U32 i = 0; i < sourceLines.size(); ++i)
	{
		std::cout << std::setw(3) << (i + 1) << ": " 
			<< sourceLines[i] << std::endl;
	}
}

//==============================================================================
void ShaderProgramPrePreprocessor::parseFileForPragmas(
	const std::string& filename, U32 depth)
{
	// first check the depth
	if(depth > MAX_DEPTH)
	{
		throw ANKI_EXCEPTION("The include depth is too high. "
			"Probably circular includance. Last: " + filename);
	}

	// load file in lines
	StringList lines = readFileLines(filename.c_str());
	if(lines.size() < 1)
	{
		throw ANKI_EXCEPTION("File is empty: " + filename);
	}

	for(const std::string& line : lines)
	{
		std::string::size_type npos;
		if(line.find(commands[ST_VERTEX]) == 0)
		{
			parseStartPragma(ST_VERTEX, line);
		}
		else if(line.find(commands[ST_TC]) == 0)
		{
			parseStartPragma(ST_TC, line);
		}
		else if(line.find(commands[ST_TE]) == 0)
		{
			parseStartPragma(ST_TE, line);
		}
		else if(line.find(commands[ST_GEOMETRY]) == 0)
		{
			parseStartPragma(ST_GEOMETRY, line);
		}
		else if(line.find(commands[ST_FRAGMENT]) == 0)
		{
			parseStartPragma(ST_FRAGMENT, line);
		}
		else if((npos = line.find(commands[5])) == 0)
		{
			std::string filen = {line, strlen(commands[5]), std::string::npos};
			filen = trimString(filen, " \"");

			parseFileForPragmas(filen, depth + 1);
		}
		else if(line.find(commands[6]) == 0)
		{
			std::string slist = {line, npos, std::string::npos};
			trffbVaryings = StringList::splitString(slist.c_str(), ' ');
		}
		else
		{
			sourceLines.push_back(line);
		}
	}
}

//==============================================================================
void ShaderProgramPrePreprocessor::parseFile(const char* filename)
{
	try
	{
		parseFileInternal(filename);
	}
	catch(Exception& e)
	{
		throw ANKI_EXCEPTION("Loading file failed: " + filename) << e;
	}
}

//==============================================================================
void ShaderProgramPrePreprocessor::parseFileInternal(const char* filename)
{
	// parse master file
	parseFileForPragmas(filename);

	// sanity checks
	if(!shaderStarts[ST_VERTEX].isDefined())
	{
		throw ANKI_EXCEPTION(ENTRYPOINT_NOT_DEFINED 
			+ commands[ST_VERTEX]);
	}

	if(!shaderStarts[ST_FRAGMENT].isDefined())
	{
		throw ANKI_EXCEPTION(ENTRYPOINT_NOT_DEFINED 
			+ commands[ST_FRAGMENT]);
	}

	// construct each shaders' source code
	for(U i = 0; i < ST_NUM; i++)
	{
		// If not defined bb
		if(!shaderStarts[i].isDefined())
		{
			continue;
		}

		std::string& src = shaderSources[i];
		src = "";

		// Sanity check: Check the correct order of i
		I32 k = (I32)i - 1;
		while(k > -1)
		{
			if(shaderStarts[k].isDefined() 
				&& shaderStarts[k].definedLine >= shaderStarts[i].definedLine)
			{
				throw ANKI_EXCEPTION(commands[i] + " must be after " 
					+ commands[k]);
			}
			--k;
		}

		k = (I32)i + 1;
		while(k < ST_NUM)
		{
			if(shaderStarts[k].isDefined() 
				&& shaderStarts[k].definedLine <= shaderStarts[i].definedLine)
			{
				throw ANKI_EXCEPTION(commands[k] + " must be after " 
					+ commands[i]);
			}
			++k;
		}

		// put global source code
		for(I32 j = 0; j < shaderStarts[ST_VERTEX].definedLine; ++j)
		{
			src += sourceLines[j] + "\n";
		}

		// put the actual code
		U32 from = i;
		U32 to;

		for(to = i + 1; to < ST_NUM; to++)
		{
			if(shaderStarts[to].definedLine != -1)
			{
				break;
			}
		}

		I32 toLine = (to == ST_NUM) ? sourceLines.size() 
			: shaderStarts[to].definedLine;

		for(I32 j = shaderStarts[from].definedLine; j < toLine; ++j)
		{
			src += sourceLines[j] + "\n";
		}
	}

	//PRINT("vertShaderBegins.globalLine: " << vertShaderBegins.globalLine)
	//PRINT("fragShaderBegins.globalLine: " << fragShaderBegins.globalLine)
	//printSourceLines();
	//printShaderVars();
}

//==============================================================================
void ShaderProgramPrePreprocessor::parseStartPragma(U32 shaderType, 
	const std::string& line)
{
	CodeBeginningPragma& pragma = shaderStarts[shaderType];

	if(pragma.definedLine != -1)
	{
		throw ANKI_EXCEPTION("Redefinition of: " + commands[shaderType]);
	}

	pragma.definedLine = sourceLines.size();
	sourceLines.push_back("//" + line);
}

} // end namespace anki
