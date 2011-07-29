#include <iomanip>
#include <cstring>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include "ShaderProgramPrePreprocessor.h"
#include "Misc/Parser.h"
#include "Util/Util.h"
#include "Util/Exception.h"


static const char* MULTIPLE_DEF_MSG = " already defined in the same place. "
	"Check for circular or multiple includance";


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
		throw EXCEPTION("File \"" + filename +
			"\": The include depth is too high. Probably circular includance");
	}

	// load file in lines
	Vec<std::string> lines = Util::getFileLines(filename.c_str());
	if(lines.size() < 1)
	{
		throw EXCEPTION("File \"" + filename + "\": Cannot open or empty");
	}

	Scanner::Scanner scanner(filename.c_str(), false);
	const Scanner::Token* token;

	while(true)
	{
		token = &scanner.getNextToken();

		// #
		if(token->getCode() == Scanner::TC_SHARP)
		{
			token = &scanner.getNextToken();
			// pragma
			if(Parser::isIdentifier(*token, "pragma"))
			{
				token = &scanner.getNextToken();
				// anki
				if(Parser::isIdentifier(*token, "anki"))
				{
					// start
					token = &scanner.getNextToken();
					if(Parser::isIdentifier(*token, "start"))
					{
						parseStartPragma(scanner, filename, depth, lines);
					}
					// include
					else if(Parser::isIdentifier(*token, "include"))
					{
						parseIncludePragma(scanner, filename, depth, lines);
					}
					// transformFeedbackVarying
					else if(Parser::isIdentifier(*token,
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
				if(token->getCode()!=Scanner::TC_NEWLINE &&
					token->getCode()!=Scanner::TC_EOF)
				{
					throw PARSER_EXCEPTION_EXPECTED("newline or end of file");
				}

				if(token->getCode() == Scanner::TC_EOF)
				{
					break;
				}

			} // end if pragma
		} // end if #
		//
		// newline
		//
		else if(token->getCode() == Scanner::TC_NEWLINE)
		{
			sourceLines.push_back(lines[scanner.getLineNumber() - 2]);
			//PRINT(lines[scanner.getLineNmbr() - 2])
		}
		//
		// EOF
		//
		else if(token->getCode() == Scanner::TC_EOF)
		{
			sourceLines.push_back(lines[scanner.getLineNumber() - 1]);
			//PRINT(lines[scanner.getLineNmbr() - 1])
			break;
		}
		//
		// error
		//
		else if(token->getCode() == Scanner::TC_ERROR)
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
		if(vertShaderBegins.globalLine == -1)
		{
			throw EXCEPTION("Entry point \"vertexShader\" is not defined");
		}
		if(fragShaderBegins.globalLine == -1)
		{
			throw EXCEPTION("Entry point \"fragmentShader\" is not defined");
		}

		// construct shader's source code
		{
			// init
			output.vertShaderSource = "";
			output.geomShaderSource = "";
			output.fragShaderSource = "";

			// put global source code
			for(int i = 0; i < vertShaderBegins.globalLine - 1; ++i)
			{
				output.vertShaderSource += sourceLines[i] + "\n";

				if(geomShaderBegins.definedInLine != -1)
				{
					output.geomShaderSource += sourceLines[i] + "\n";
				}

				output.fragShaderSource += sourceLines[i] + "\n";
			}

			// vert shader code
			int limit = (geomShaderBegins.definedInLine != -1) ?
				geomShaderBegins.globalLine-1 : fragShaderBegins.globalLine-1;
			for(int i = vertShaderBegins.globalLine - 1; i < limit; ++i)
			{
				output.vertShaderSource += sourceLines[i] + "\n";
			}

			// geom shader code
			if(geomShaderBegins.definedInLine != -1)
			{
				for(int i = geomShaderBegins.globalLine - 1;
					i < fragShaderBegins.globalLine - 1; ++i)
				{
					output.geomShaderSource += sourceLines[i] + "\n";
				}
			}

			// frag shader code
			for(int i = fragShaderBegins.globalLine - 1;
				i < int(sourceLines.size()); ++i)
			{
				output.fragShaderSource += sourceLines[i] + "\n";
			}
		} // end construct shader's source code

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
		throw EXCEPTION("Started from \"" + filename + "\": " + e.what());
	}
}


//==============================================================================
// parseStartPragma                                                            =
//==============================================================================
void ShaderProgramPrePreprocessor::parseStartPragma(Scanner::Scanner& scanner,
	const std::string& filename, uint depth, const Vec<std::string>& lines)
{
	const Scanner::Token* token = &scanner.getNextToken();

	// Chose the correct pragma
	CodeBeginningPragma* cbp;
	std::string name;
	if(Parser::isIdentifier(*token, "vertexShader"))
	{
		cbp = &vertShaderBegins;
		name = "vertexShader";
	}
	else if(Parser::isIdentifier(*token, "geometryShader"))
	{
		cbp = &geomShaderBegins;
		name = "geometryShader";
	}
	else if(Parser::isIdentifier(*token, "fragmentShader"))
	{
		cbp = &fragShaderBegins;
		name = "fragmentShader";
	}
	else
	{
		throw PARSER_EXCEPTION_EXPECTED("vertexShader or geometryShader or "
			"fragmentShader");
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
	sourceLines.push_back("#line " +
		boost::lexical_cast<std::string>(scanner.getLineNumber()) +
		' ' + boost::lexical_cast<std::string>(depth) +
		" // " + lines[scanner.getLineNumber() - 1]);
}


//==============================================================================
// parseIncludePragma                                                          =
//==============================================================================
void ShaderProgramPrePreprocessor::parseIncludePragma(
	Scanner::Scanner& scanner, const std::string& /*filename*/, uint depth,
	const Vec<std::string>& lines)
{
	const Scanner::Token* token = &scanner.getNextToken();

	if(token->getCode() == Scanner::TC_STRING)
	{
		std::string filename = token->getValue().getString();

		//int line = sourceLines.size();
		sourceLines.push_back("#line 0 " +
			boost::lexical_cast<std::string>(depth + 1) +
			" // " + lines[scanner.getLineNumber() - 1]);

		parseFileForPragmas(filename.c_str(), depth + 1);

		sourceLines.push_back("#line " +
			boost::lexical_cast<std::string>(scanner.getLineNumber()) + ' ' +
			boost::lexical_cast<std::string>(depth) + " // end of " +
			lines[scanner.getLineNumber() - 1]);
	}
	else
	{
		throw PARSER_EXCEPTION_EXPECTED("string");
	}
}


//==============================================================================
// parseTrffbVarying                                                           =
//==============================================================================
void ShaderProgramPrePreprocessor::parseTrffbVarying(Scanner::Scanner& scanner,
	const std::string& filename, uint /*depth*/, const Vec<std::string>& lines)
{
	const Scanner::Token* token = &scanner.getNextToken();

	if(token->getCode() == Scanner::TC_IDENTIFIER)
	{
		std::string varName = token->getValue().getString();

		// check if already defined and for circular includance
		Vec<TrffbVaryingPragma>::const_iterator var =
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

