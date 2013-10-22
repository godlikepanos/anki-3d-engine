#include "anki/resource/ShaderProgramPrePreprocessor.h"
#include "anki/resource/ResourceManager.h"
#include "anki/util/Exception.h"
#include "anki/util/Functions.h"
#include "anki/util/File.h"
#include <iomanip>
#include <cstring>
#include <iostream>

namespace anki {

//==============================================================================

// Keep the strings in that order so that the start pragmas will match to the
// ShaderType enums
static Array<const char*, 10> commands = {{
	"#pragma anki start vertexShader",
	"#pragma anki start tcShader",
	"#pragma anki start teShader",
	"#pragma anki start geometryShader",
	"#pragma anki start fragmentShader",
	"#pragma anki start computeShader",
	"#pragma anki include",
	"#pragma anki transformFeedbackVaryings separate",
	"#pragma anki transformFeedbackVaryings interleaved"
	"#pragma anki disable tess"}};

static_assert(ST_VERTEX == 0 && ST_COMPUTE == 5, "See file");

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
	StringList lines;
	File(filename.c_str(), File::OF_READ).readAllTextLines(lines);
	if(lines.size() < 1)
	{
		throw ANKI_EXCEPTION("File is empty: " + filename);
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
		else if(line.find(commands[ST_COMPUTE]) == 0)
		{
			parseStartPragma(ST_COMPUTE, line);
		}
		else if((npos = line.find(commands[6])) == 0)
		{
			std::string filen = {line, strlen(commands[6]), std::string::npos};
			filen = trimString(filen, " \"");

			filen = 
				ResourceManagerSingleton::get().fixResourcePath(filen.c_str());

			parseFileForPragmas(filen, depth + 1);
		}
		else if((npos = line.find(commands[7])) == 0)
		{
			std::string slist = {line, strlen(commands[7]), std::string::npos};
			trffbVaryings = StringList::splitString(slist.c_str(), ' ');
			xfbBufferMode = XFBBM_SEPARATE;
		}
		else if((npos = line.find(commands[8])) == 0)
		{
			std::string slist = {line, strlen(commands[8]), std::string::npos};
			trffbVaryings = StringList::splitString(slist.c_str(), ' ');
			xfbBufferMode = XFBBM_INTERLEAVED;
		}
		else if((npos = line.find(commands[9])) == 0)
		{
			enableTess = false;	
		}
		else
		{
			gotPragmaAnki = false;
			sourceLines.push_back(line);
		}

		// Sanity check
		if(expectPragmaAnki && !gotPragmaAnki)
		{
			throw ANKI_EXCEPTION("Malformed pragma anki: " + line);
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
	ShaderType firstShader; // Before that shader there is global code

	// Parse master file
	parseFileForPragmas(filename);

	// Sanity checks and some calculations
	if(!shaderStarts[ST_COMPUTE].isDefined())
	{
		// If not compute then vertex and fragment shaders should be defined

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

		firstShader = ST_VERTEX;
	}
	else
	{
		// If compute is defined everything else should not be

		if(shaderStarts[ST_VERTEX].isDefined() 
			|| shaderStarts[ST_TE].isDefined()
			|| shaderStarts[ST_TC].isDefined()
			|| shaderStarts[ST_FRAGMENT].isDefined()
			|| shaderStarts[ST_GEOMETRY].isDefined())
		{
			throw ANKI_EXCEPTION("Compute shader should be alone");
		}

		firstShader = ST_COMPUTE;
	}

	// Construct each shaders' source code
	for(U i = 0; i < ST_NUM; i++)
	{
		// If not defined bb
		if(!shaderStarts[i].isDefined())
		{
			continue;
		}

		std::string& src = shaderSources[i];
		src = "";

		// The i-nth shader should be defined after the [0, i-1]
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

		// And shouldn't be defined before the [i+1, ST_NUM-1]
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

		// Append the global source code
		ANKI_ASSERT(shaderStarts[firstShader].definedLine != -1);
		for(I32 j = 0; j < shaderStarts[firstShader].definedLine; ++j)
		{
			src += sourceLines[j] + "\n";
		}

		// Put the actual code
		U32 from = i;
		U32 to;

		// Calculate the "to". If i is vertex the to could be anything
		for(to = i + 1; to < ST_NUM; to++)
		{
			if(shaderStarts[to].definedLine != -1)
			{
				break;
			}
		}

		I32 toLine = (to != ST_NUM) ? shaderStarts[to].definedLine
			: sourceLines.size();

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
