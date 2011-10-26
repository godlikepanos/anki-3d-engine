#include "anki/resource/ShaderProgramPrePreprocessor.h"
#include "anki/misc/Parser.h"
#include "anki/util/Util.h"
#include "anki/util/Exception.h"
#include <iomanip>
#include <cstring>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>


namespace anki {


static const char* MULTIPLE_DEF_MSG = " already defined in the same place. "
	"Check for circular or multiple includance";


boost::array<const char*, ST_NUM>
	ShaderProgramPrePreprocessor::startTokens = {{
	"vertexShader",
	"tcShader",
	"teShader",
	"geometryShader",
	"fragmentShader"}};


//==============================================================================
// printSourceLines                                                            =
//==============================================================================
void ShaderProgramPrePreprocessor::printSourceLines() const
{
	for(uint i = 0; i < sourceLines.size(); ++i)
	{
		std::cout << std::setw(3) << (i + 1) << ": " <<
			sourceLines[i] << std::endl;
	}
}


//==============================================================================
// parseFileForPragmas                                                         =
//==============================================================================
void ShaderProgramPrePreprocessor::parseFileForPragmas(
	const std::string& filename, int depth)
{
	// first check the depth
	if(depth > 99)
	{
		throw ANKI_EXCEPTION("File \"" + filename +
			"\": The include depth is too high. Probably circular includance");
	}

	// load file in lines
	std::vector<std::string> lines = Util::getFileLines(filename.c_str());
	if(lines.size() < 1)
	{
		throw ANKI_EXCEPTION("File \"" + filename + "\": Cannot open or empty");
	}

	scanner::Scanner scanner(filename.c_str(), false);
	const scanner::Token* token;

	while(true)
	{
		token = &scanner.getNextToken();

		// #
		if(token->getCode() == scanner::TC_SHARP)
		{
			token = &scanner.getNextToken();
			// pragma
			if(parser::isIdentifier(*token, "pragma"))
			{
				token = &scanner.getNextToken();
				// anki
				if(parser::isIdentifier(*token, "anki"))
				{
					// start
					token = &scanner.getNextToken();
					if(parser::isIdentifier(*token, "start"))
					{
						parseStartPragma(scanner, filename, depth, lines);
					}
					// include
					else if(parser::isIdentifier(*token, "include"))
					{
						parseIncludePragma(scanner, filename, depth, lines);
					}
					// transformFeedbackVarying
					else if(parser::isIdentifier(*token,
						"transformFeedbackVarying"))
					{
						parseTrffbVarying(scanner, filename, depth, lines);
					}
					// error
					else
					{
						throw PARSER_EXCEPTION(
							"#pragma anki followed by incorrect " +
							token->getInfoStr());
					}
				} // end if anki

				token = &scanner.getNextToken();
				if(token->getCode()!=scanner::TC_NEWLINE &&
					token->getCode()!=scanner::TC_END)
				{
					throw PARSER_EXCEPTION_EXPECTED("newline or end of file");
				}

				if(token->getCode() == scanner::TC_END)
				{
					break;
				}

			} // end if pragma
		} // end if #
		//
		// newline
		//
		else if(token->getCode() == scanner::TC_NEWLINE)
		{
			sourceLines.push_back(lines[scanner.getLineNumber() - 2]);
			//PRINT(lines[scanner.getLineNmbr() - 2])
		}
		//
		// EOF
		//
		else if(token->getCode() == scanner::TC_END)
		{
			sourceLines.push_back(lines[scanner.getLineNumber() - 1]);
			//PRINT(lines[scanner.getLineNmbr() - 1])
			break;
		}
		//
		// error
		//
		else if(token->getCode() == scanner::TC_ERROR)
		{
			// It will never get here
		}
	} // end while
}


//=============================================================================/
// parseFile                                                                   =
//==============================================================================
void ShaderProgramPrePreprocessor::parseFile(const char* filename)
{
	try
	{
		// parse master file
		parseFileForPragmas(filename);
	
		// sanity checks
		if(!shaderStarts[ST_VERTEX].isDefined())
		{
			throw ANKI_EXCEPTION("Entry point \""+ startTokens[ST_VERTEX] +
				"\" is not defined");
		}

		if(!shaderStarts[ST_FRAGMENT].isDefined())
		{
			throw ANKI_EXCEPTION("Entry point \""+ startTokens[ST_FRAGMENT] +
				"\" is not defined");
		}

		// construct shaders' source code
		for(uint i = 0; i < ST_NUM; i++)
		{
			std::string& src = output.shaderSources[i];

			src = "";

			// If not defined bb
			if(!shaderStarts[i].isDefined())
			{
				continue;
			}

			// Sanity check: Check the correct order of i
			int k = (int)i - 1;
			while(k > -1)
			{
				if(shaderStarts[k].isDefined() &&
					shaderStarts[k].globalLine >= shaderStarts[i].globalLine)
				{
					throw ANKI_EXCEPTION(startTokens[i] + " must be after " +
						startTokens[k]);
				}
				--k;
			}

			k = (int)i + 1;
			while(k < ST_NUM)
			{
				if(shaderStarts[k].isDefined() &&
					shaderStarts[k].globalLine <= shaderStarts[i].globalLine)
				{
					throw ANKI_EXCEPTION(startTokens[k] + " must be after " +
						startTokens[i]);
				}
				++k;
			}

			// put global source code
			for(int j = 0; j < shaderStarts[ST_VERTEX].globalLine - 1; ++j)
			{
				src += sourceLines[j] + "\n";
			}

			// put the actual code
			uint from = i;
			uint to;

			for(to = i + 1; to < ST_NUM; to++)
			{
				if(shaderStarts[to].definedInLine != -1)
				{
					break;
				}
			}

			int toLine = (to == ST_NUM) ? sourceLines.size():
				shaderStarts[to].globalLine - 1;

			for(int j = shaderStarts[from].globalLine - 1; j < toLine; ++j)
			{
				src += sourceLines[j] + "\n";
			}
		}

		// TRF feedback varyings
		BOOST_FOREACH(const TrffbVaryingPragma& trffbv, output.trffbVaryings)
		{
			trffbVaryings.push_back(trffbv.name);
		}

		//PRINT("vertShaderBegins.globalLine: " << vertShaderBegins.globalLine)
		//PRINT("fragShaderBegins.globalLine: " << fragShaderBegins.globalLine)
		//printSourceLines();
		//printShaderVars();
	}
	catch(Exception& e)
	{
		throw ANKI_EXCEPTION("Started from \"" + filename + "\": " + e.what());
	}
}


//==============================================================================
// parseStartPragma                                                            =
//==============================================================================
void ShaderProgramPrePreprocessor::parseStartPragma(scanner::Scanner& scanner,
	const std::string& filename, uint depth,
	const std::vector<std::string>& lines)
{
	const scanner::Token* token = &scanner.getNextToken();

	// Chose the correct pragma
	CodeBeginningPragma* cbp = NULL;
	const char* name = NULL;

	for(uint i = 0; i < ST_NUM; i++)
	{
		if(parser::isIdentifier(*token, startTokens[i]))
		{
			cbp = &shaderStarts[i];
			name = startTokens[i];
			break;
		}
	}

	if(name == NULL)
	{
		throw PARSER_EXCEPTION_UNEXPECTED();
	}

	// its defined in same place so there is probable circular includance
	if(cbp->definedInLine == scanner.getLineNumber() &&
		cbp->definedInFile == filename)
	{
		throw PARSER_EXCEPTION(name + MULTIPLE_DEF_MSG);
	}

	// already defined elsewhere => error
	if(cbp->definedInLine != -1)
	{
		throw PARSER_EXCEPTION( name +  " already defined at " +
			cbp->definedInFile + ":" +
			boost::lexical_cast<std::string>(cbp->definedInLine));
	}

	cbp->definedInFile = filename;
	cbp->definedInLine = scanner.getLineNumber();
	cbp->globalLine = sourceLines.size() + 1;
	addLinePreProcExpression(scanner.getLineNumber(), depth,
		lines[scanner.getLineNumber() - 1].c_str());
}


//==============================================================================
// parseIncludePragma                                                          =
//==============================================================================
void ShaderProgramPrePreprocessor::parseIncludePragma(
	scanner::Scanner& scanner, const std::string& /*filename*/, uint depth,
	const std::vector<std::string>& lines)
{
	const scanner::Token* token = &scanner.getNextToken();

	if(token->getCode() == scanner::TC_STRING)
	{
		std::string filename = token->getValue().getString();

		//int line = sourceLines.size();
		addLinePreProcExpression(0, depth + 1,
			lines[scanner.getLineNumber() - 1].c_str());

		parseFileForPragmas(filename.c_str(), depth + 1);

		addLinePreProcExpression(scanner.getLineNumber(), depth,
			(" // end of " + lines[scanner.getLineNumber() - 1]).c_str());
	}
	else
	{
		throw PARSER_EXCEPTION_EXPECTED("string");
	}
}


//==============================================================================
// parseTrffbVarying                                                           =
//==============================================================================
void ShaderProgramPrePreprocessor::parseTrffbVarying(scanner::Scanner& scanner,
	const std::string& filename, uint /*depth*/,
	const std::vector<std::string>& lines)
{
	const scanner::Token* token = &scanner.getNextToken();

	if(token->getCode() == scanner::TC_IDENTIFIER)
	{
		std::string varName = token->getValue().getString();

		// check if already defined and for circular includance
		std::vector<TrffbVaryingPragma>::const_iterator var =
			findNamed(output.trffbVaryings, varName);

		// Throw the correct exception
		if(var != output.trffbVaryings.end())
		{
			if(var->definedInLine == scanner.getLineNumber() &&
				var->definedInFile == filename)
			{
				throw PARSER_EXCEPTION("\"" + varName +
					"\"" + MULTIPLE_DEF_MSG);
			}
			else
			{
				throw PARSER_EXCEPTION("Varying \"" + varName +
					"\" already defined at " + var->definedInFile + ":" +
					boost::lexical_cast<std::string>(var->definedInLine));
			}
		}

		// all ok, push it back
		output.trffbVaryings.push_back(TrffbVaryingPragma(filename,
			scanner.getLineNumber(), varName));

		sourceLines.push_back(lines[scanner.getLineNumber() - 1]);
	}
	else
	{
		throw PARSER_EXCEPTION_EXPECTED("identifier");
	}
}


//==============================================================================
// addLinePreProcExpression                                                    =
//==============================================================================
void ShaderProgramPrePreprocessor::addLinePreProcExpression(
	uint /*line*/, uint /*depth*/, const char* /*cmnt*/)
{
	/*sourceLines.push_back("#line " +
		boost::lexical_cast<std::string>(line) +
		' ' +
		boost::lexical_cast<std::string>(depth) +
		" // " +
		cmnt);*/
}


} // end namespace
