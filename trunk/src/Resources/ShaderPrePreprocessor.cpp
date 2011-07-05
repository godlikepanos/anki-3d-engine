#include <iomanip>
#include <cstring>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include "ShaderPrePreprocessor.h"
#include "Misc/Parser.h"
#include "Util/Util.h"
#include "Util/Exception.h"


static const char* MULTIPLE_DEF_MSG = " already defined in the same place. Check for circular or multiple includance";


//==============================================================================
// printSourceLines                                                            =
//==============================================================================
void ShaderPrePreprocessor::printSourceLines() const
{
	for(uint i = 0; i < sourceLines.size(); ++i)
	{
		std::cout << std::setw(3) << (i + 1) << ": " <<
			sourceLines[i] << std::endl;
	}
}


//==============================================================================
// printShaderVars                                                             =
//==============================================================================
void ShaderPrePreprocessor::printShaderVars() const
{
	std::cout << "TYPE" << std::setw(20) << "NAME" << std::setw(4) <<
		"LOC" << std::endl;
	for(uint i = 0; i < output.attributes.size(); ++i)
	{
		std::cout << std::setw(4) << "A" << std::setw(20) <<
			output.attributes[i].name << std::setw(4) <<
			output.attributes[i].customLoc << std::endl;
	}
}


//==============================================================================
// findShaderVar                                                               =
//==============================================================================
Vec<ShaderPrePreprocessor::ShaderVarPragma>::iterator
	ShaderPrePreprocessor::findShaderVar(Vec<ShaderVarPragma>& vec,
	const std::string& name) const
{
	Vec<ShaderVarPragma>::iterator it = vec.begin();
	while(it != vec.end() && it->name != name)
	{
		++it;
	}
	return it;
}


//==============================================================================
// parseFileForPragmas                                                         =
//==============================================================================
void ShaderPrePreprocessor::parseFileForPragmas(const std::string& filename,
	int depth)
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

		if(token->getCode() == Scanner::TC_SHARP)
		{
			token = &scanner.getNextToken();
			if(token->getCode() == Scanner::TC_IDENTIFIER &&
				strcmp(token->getValue().getString(), "pragma") == 0)
			{
				token = &scanner.getNextToken();
				if(token->getCode() == Scanner::TC_IDENTIFIER &&
					strcmp(token->getValue().getString(), "anki") == 0)
				{
					token = &scanner.getNextToken();
					//
					// vertShaderBegins
					//
					if(token->getCode() == Scanner::TC_IDENTIFIER &&
						strcmp(token->getValue().getString(),
						"vertShaderBegins") == 0)
					{
						// play

						// its defined in same place so there is probable
						// circular includance
						if(vertShaderBegins.definedInLine ==
							scanner.getLineNumber() &&
							vertShaderBegins.definedInFile == filename)
						{
							throw PARSER_EXCEPTION("vertShaderBegins" +
								MULTIPLE_DEF_MSG);
						}

						// already defined elsewhere => error
						if(vertShaderBegins.definedInLine != -1)
						{
							throw PARSER_EXCEPTION(
								"vertShaderBegins already defined at " +
								vertShaderBegins.definedInFile + ":" +
								boost::lexical_cast<std::string>(
								vertShaderBegins.definedInLine));
						}

						// vert shader should be before frag
						if(fragShaderBegins.definedInLine != -1)
						{
							throw PARSER_EXCEPTION(
								"vertShaderBegins must precede "
								"fragShaderBegins defined at " +
								fragShaderBegins.definedInFile + ":" +
								boost::lexical_cast<std::string>(
								fragShaderBegins.definedInLine));
						}

						// vert shader should be before geom
						if(geomShaderBegins.definedInLine != -1)
						{
							throw PARSER_EXCEPTION(
								"vertShaderBegins must precede "
								"geomShaderBegins defined at " +
								geomShaderBegins.definedInFile + ":" +
								boost::lexical_cast<std::string>(
								geomShaderBegins.definedInLine));
						}

						vertShaderBegins.definedInFile = filename;
						vertShaderBegins.definedInLine =
							scanner.getLineNumber();
						vertShaderBegins.globalLine = sourceLines.size() + 1;
						sourceLines.push_back("#line " +
							boost::lexical_cast<std::string>(
							scanner.getLineNumber()) +
							' ' + boost::lexical_cast<std::string>(depth) +
							" // " + lines[scanner.getLineNumber()-1]);
						// stop play
					}
					//
					// geomShaderBegins
					//
					else if(token->getCode() == Scanner::TC_IDENTIFIER &&
						strcmp(token->getValue().getString(),
						"geomShaderBegins") == 0)
					{
						// play

						// its defined in same place so there is probable circular includance
						if(geomShaderBegins.definedInLine ==
							scanner.getLineNumber() &&
							geomShaderBegins.definedInFile == filename)
						{
							throw PARSER_EXCEPTION("geomShaderBegins" +
								MULTIPLE_DEF_MSG);
						}

						// already defined elsewhere => error
						if(geomShaderBegins.definedInLine != -1)
						{
							throw PARSER_EXCEPTION(
								"geomShaderBegins already defined at " +
								geomShaderBegins.definedInFile + ":" +
								boost::lexical_cast<std::string>(
								geomShaderBegins.definedInLine));
						}

						// vert shader entry point not defined => error
						if(vertShaderBegins.definedInLine == -1)
						{
							throw PARSER_EXCEPTION(
								"geomShaderBegins must follow "
								"vertShaderBegins");
						}

						// frag shader entry point defined => error
						if(fragShaderBegins.definedInLine != -1)
						{
							throw PARSER_EXCEPTION(
								"geomShaderBegins must precede"
								" fragShaderBegins defined at " +
								fragShaderBegins.definedInFile + ":" +
								boost::lexical_cast<std::string>(
								fragShaderBegins.definedInLine));
						}

						geomShaderBegins.definedInFile = filename;
						geomShaderBegins.definedInLine =
							scanner.getLineNumber();
						geomShaderBegins.globalLine = sourceLines.size() + 1;
						sourceLines.push_back("#line " +
							boost::lexical_cast<std::string>(
							scanner.getLineNumber()) + ' ' +
							boost::lexical_cast<std::string>(depth) + " // " +
							lines[scanner.getLineNumber()-1]);
						// stop play
					}
					//
					// fragShaderBegins
					//
					else if(token->getCode() == Scanner::TC_IDENTIFIER &&
						strcmp(token->getValue().getString(),
						"fragShaderBegins") == 0)
					{
						// play

						// its defined in same place so there is probable
						// circular includance
						if(fragShaderBegins.definedInLine ==
							scanner.getLineNumber() &&
							fragShaderBegins.definedInFile == filename)
						{
							throw PARSER_EXCEPTION("fragShaderBegins" +
								MULTIPLE_DEF_MSG);
						}

						// if already defined elsewhere throw error
						if(fragShaderBegins.definedInLine != -1)
						{
							throw PARSER_EXCEPTION(
								"fragShaderBegins already defined at " +
								fragShaderBegins.definedInFile + ":" +
								boost::lexical_cast<std::string>(
								fragShaderBegins.definedInLine));
						}

						// vert shader entry point not defined
						if(vertShaderBegins.definedInLine == -1)
						{
							throw PARSER_EXCEPTION(
							"fragShaderBegins must follow vertShaderBegins");
						}

						fragShaderBegins.definedInFile = filename;
						fragShaderBegins.definedInLine =
							scanner.getLineNumber();
						fragShaderBegins.globalLine = sourceLines.size() + 1;
						sourceLines.push_back("#line " +
							boost::lexical_cast<std::string>(
							scanner.getLineNumber()) + ' ' +
							boost::lexical_cast<std::string>(depth) + " // " +
							lines[scanner.getLineNumber()-1]);
						// stop play
					}
					//
					// include
					//
					else if(token->getCode() == Scanner::TC_IDENTIFIER &&
						strcmp(token->getValue().getString(), "include") == 0)
					{
						token = &scanner.getNextToken();
						if(token->getCode() == Scanner::TC_STRING)
						{
							// play
							//int line = sourceLines.size();
							sourceLines.push_back("#line 0 " +
								boost::lexical_cast<std::string>(depth + 1) +
								" // " + lines[scanner.getLineNumber() - 1]);
							parseFileForPragmas(token->getValue().getString(),
								depth + 1);
							sourceLines.push_back("#line " +
								boost::lexical_cast<std::string>(
								scanner.getLineNumber()) + ' ' +
								boost::lexical_cast<std::string>(depth) +
								" // end of " +
								lines[scanner.getLineNumber() - 1]);
							// stop play
						}
						else
						{
							throw PARSER_EXCEPTION_EXPECTED("string");
						}
					}
					//
					// transformFeedbackVarying
					//
					else if(token->getCode() == Scanner::TC_IDENTIFIER &&
						strcmp(token->getValue().getString(),
						"transformFeedbackVarying") == 0)
					{
						token = &scanner.getNextToken();
						if(token->getCode() == Scanner::TC_IDENTIFIER)
						{
							std::string varName = token->getValue().getString();
							// check if already defined and for circular
							// includance
							Vec<TrffbVaryingPragma>::const_iterator var =
								findNamed(output.trffbVaryings, varName);
							if(var != output.trffbVaryings.end())
							{
								if(var->definedInLine ==
									scanner.getLineNumber() &&
									var->definedInFile==filename)
								{
									throw PARSER_EXCEPTION("\"" + varName +
										"\"" + MULTIPLE_DEF_MSG);
								}
								else
								{
									throw PARSER_EXCEPTION("Varying \"" +
										varName + "\" already defined at " +
										var->definedInFile + ":" +
										boost::lexical_cast<std::string>(
										var->definedInLine));
								}
							}

							// all ok, push it back
							output.trffbVaryings.push_back(TrffbVaryingPragma(filename, scanner.getLineNumber(),
							                                                  varName));
							sourceLines.push_back(lines[scanner.getLineNumber() - 1]);
						}
						else
						{
							throw PARSER_EXCEPTION_EXPECTED("identifier");
						}
					}
					//
					// attribute
					//
					else if(token->getCode() == Scanner::TC_IDENTIFIER &&
					        strcmp(token->getValue().getString(), "attribute") == 0)
					{
						throw EXCEPTION("Deprecated feature");

						token = &scanner.getNextToken();
						if(token->getCode() == Scanner::TC_IDENTIFIER)
						{
							std::string varName = token->getValue().getString();
							token = &scanner.getNextToken();
							if(token->getCode() == Scanner::TC_NUMBER && token->getDataType() == Scanner::DT_INT)
							{
								uint loc = token->getValue().getInt();

								// check if already defined and for circular includance
								Vec<ShaderVarPragma>::const_iterator attrib = findNamed(output.attributes, varName);
								if(attrib != output.attributes.end())
								{
									if(attrib->definedInLine == scanner.getLineNumber() &&
									   attrib->definedInFile == filename)
									{
										throw PARSER_EXCEPTION("\"" + varName + "\"" + MULTIPLE_DEF_MSG);
									}
									else
									{
										throw PARSER_EXCEPTION("Attribute \"" + varName + "\" already defined at " +
										                       attrib->definedInFile + ":" +
										                       boost::lexical_cast<std::string>(attrib->definedInLine));
									}
								}
								// search if another var has the same loc
								for(attrib = output.attributes.begin(); attrib != output.attributes.end(); ++attrib)
								{
									if(attrib->customLoc == loc)
									{
										throw PARSER_EXCEPTION("The attributes \"" + attrib->name + "\" (" +
										                       attrib->definedInFile + ":" +
										                       boost::lexical_cast<std::string>(attrib->definedInLine) +
										                       ") and \"" + varName + "\" share the same location");
									}
								}

								// all ok, push it back
								output.attributes.push_back(ShaderVarPragma(filename, scanner.getLineNumber(),
								                                            varName, loc));
								sourceLines.push_back(lines[scanner.getLineNumber() - 1]);
							}
							else
							{
								throw PARSER_EXCEPTION_EXPECTED("integer");
							}
						}
						else
						{
							throw PARSER_EXCEPTION_EXPECTED("identifier");
						}
					}
					else
					{
						throw PARSER_EXCEPTION("#pragma anki followed by incorrect " + token->getInfoStr());
					}
				} // end if anki

				token = &scanner.getNextToken();
				if(token->getCode()!=Scanner::TC_NEWLINE && token->getCode()!=Scanner::TC_EOF)
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


//==============================================================================
// parseFile                                                                   =
//==============================================================================
void ShaderPrePreprocessor::parseFile(const char* filename)
{
	try
	{
		// parse master file
		parseFileForPragmas(filename);


		// rearrange the trffbVaryings
	
		// sanity checks
		if(vertShaderBegins.globalLine == -1)
		{
			throw EXCEPTION("Entry point \"vertShaderBegins\" is not defined");
		}
		if(fragShaderBegins.globalLine == -1)
		{
			throw EXCEPTION("Entry point \"fragShaderBegins\" is not defined");
		}

		// construct shader's source code
		{
			// init
			output.vertShaderSource = "";
			output.geomShaderSource = "";
			output.fragShaderSource = "";

			// put global source code
			for(int i=0; i<vertShaderBegins.globalLine-1; ++i)
			{
				output.vertShaderSource += sourceLines[i] + "\n";

				if(geomShaderBegins.definedInLine != -1)
				{
					output.geomShaderSource += sourceLines[i] + "\n";
				}

				output.fragShaderSource += sourceLines[i] + "\n";
			}

			// vert shader code
			int limit = (geomShaderBegins.definedInLine != -1) ? geomShaderBegins.globalLine-1 :
			                                                     fragShaderBegins.globalLine-1;
			for(int i = vertShaderBegins.globalLine - 1; i < limit; ++i)
			{
				output.vertShaderSource += sourceLines[i] + "\n";
			}

			// geom shader code
			if(geomShaderBegins.definedInLine != -1)
			{
				for(int i = geomShaderBegins.globalLine - 1; i < fragShaderBegins.globalLine - 1; ++i)
				{
					output.geomShaderSource += sourceLines[i] + "\n";
				}
			}

			// frag shader code
			for(int i = fragShaderBegins.globalLine - 1; i < int(sourceLines.size()); ++i)
			{
				output.fragShaderSource += sourceLines[i] + "\n";
			}
		} // end construct shader's source code

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

