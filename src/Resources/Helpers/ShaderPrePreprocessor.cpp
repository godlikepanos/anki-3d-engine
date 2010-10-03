#include <iomanip>
#include <cstring>
#include <boost/lexical_cast.hpp>
#include "ShaderPrePreprocessor.h"
#include "Scanner.h"
#include "Parser.h"
#include "Util.h"

const char* MULTIPLE_DEF_MSG = " already defined in the same place. Check for circular or multiple includance";


//======================================================================================================================
// printSourceLines                                                                                                    =
//======================================================================================================================
void ShaderPrePreprocessor::printSourceLines() const
{
	for(uint i=0; i<sourceLines.size(); ++i)
	{
		PRINT(setw(3) << i+1 << ": " << sourceLines[i]);
	}
}


//======================================================================================================================
// printShaderVars                                                                                                     =
//======================================================================================================================
void ShaderPrePreprocessor::printShaderVars() const
{
	PRINT("TYPE" << setw(20) << "NAME" << setw(4) << "LOC");
	for(uint i=0; i<output.attributes.size(); ++i)
	{
		PRINT(setw(4) << "A" << setw(20) << output.attributes[i].name << setw(4) << output.attributes[i].customLoc);
	}
}


//======================================================================================================================
// findShaderVar                                                                                                       =
//======================================================================================================================
Vec<ShaderPrePreprocessor::ShaderVarPragma>::iterator ShaderPrePreprocessor::findShaderVar(Vec<ShaderVarPragma>& vec,
                                                                                           const string& name) const
{
	Vec<ShaderVarPragma>::iterator it = vec.begin();
	while(it != vec.end() && it->name != name)
	{
		++it;
	}
	return it;
}


//======================================================================================================================
// parseFileForPragmas                                                                                                 =
//======================================================================================================================
bool ShaderPrePreprocessor::parseFileForPragmas(const string& filename, int depth)
{
	// first check the depth
	if(depth > 99)
	{
		ERROR("File \"" << filename << "\": The include depth is too high. Probably circular includance");
		return false;
	}

	// load file in lines
	Vec<string> lines = Util::getFileLines(filename.c_str());
	if(lines.size() < 1)
	{
		ERROR("File \"" << filename << "\": Cannot open or empty");
		return false;
	}

	// scanner
	try
	{
		Scanner scanner(filename.c_str(), false);
		const Scanner::Token* token;

		while(true)
		{
			token = &scanner.getNextToken();

			if(token->getCode() == Scanner::TC_SHARP)
			{
				token = &scanner.getNextToken();
				if(token->getCode() == Scanner::TC_IDENTIFIER && strcmp(token->getValue().getString(), "pragma") == 0)
				{
					token = &scanner.getNextToken();
					if(token->getCode() == Scanner::TC_IDENTIFIER && strcmp(token->getValue().getString(), "anki") == 0)
					{
						token = &scanner.getNextToken();
						//
						// vertShaderBegins
						//
						if(token->getCode() == Scanner::TC_IDENTIFIER &&
							 strcmp(token->getValue().getString(), "vertShaderBegins") == 0)
						{
							// play

							// its defined in same place so there is probable circular includance
							if(vertShaderBegins.definedInLine==scanner.getLineNumber() && vertShaderBegins.definedInFile==filename)
							{
								PARSE_ERR("vertShaderBegins" << MULTIPLE_DEF_MSG);
								return false;
							}

							// already defined elsewhere => error
							if(vertShaderBegins.definedInLine != -1)
							{
								PARSE_ERR("vertShaderBegins already defined at " << vertShaderBegins.definedInFile << ":" <<
													 vertShaderBegins.definedInLine);
								return false;
							}

							// vert shader should be before frag
							if(fragShaderBegins.definedInLine != -1)
							{
								PARSE_ERR("vertShaderBegins must precede fragShaderBegins defined at " <<
													fragShaderBegins.definedInFile << ":" << fragShaderBegins.definedInLine);
								return false;
							}

							// vert shader should be before geom
							if(geomShaderBegins.definedInLine != -1)
							{
								PARSE_ERR("vertShaderBegins must precede geomShaderBegins defined at " <<
													geomShaderBegins.definedInFile << ":" << geomShaderBegins.definedInLine);
								return false;
							}

							vertShaderBegins.definedInFile = filename;
							vertShaderBegins.definedInLine = scanner.getLineNumber();
							vertShaderBegins.globalLine = sourceLines.size() + 1;
							sourceLines.push_back("#line " + lexical_cast<string>(scanner.getLineNumber()) + ' ' +
																		lexical_cast<string>(depth) + " // " + lines[scanner.getLineNumber()-1]);
							// stop play
						}
						//
						// geomShaderBegins
						//
						else if(token->getCode() == Scanner::TC_IDENTIFIER &&
										strcmp(token->getValue().getString(), "geomShaderBegins") == 0)
						{
							// play

							// its defined in same place so there is probable circular includance
							if(geomShaderBegins.definedInLine==scanner.getLineNumber() && geomShaderBegins.definedInFile==filename)
							{
								PARSE_ERR("geomShaderBegins" << MULTIPLE_DEF_MSG);
								return false;
							}

							// already defined elsewhere => error
							if(geomShaderBegins.definedInLine != -1)
							{
								PARSE_ERR("geomShaderBegins already defined at " << geomShaderBegins.definedInFile << ":" <<
													 geomShaderBegins.definedInLine);
								return false;
							}

							// vert shader entry point not defined => error
							if(vertShaderBegins.definedInLine == -1)
							{
								PARSE_ERR("geomShaderBegins must follow vertShaderBegins");
								return false;
							}

							// frag shader entry point defined => error
							if(fragShaderBegins.definedInLine != -1)
							{
								PARSE_ERR("geomShaderBegins must precede fragShaderBegins defined at " <<
													fragShaderBegins.definedInFile << ":" << fragShaderBegins.definedInLine);
								return false;
							}

							geomShaderBegins.definedInFile = filename;
							geomShaderBegins.definedInLine = scanner.getLineNumber();
							geomShaderBegins.globalLine = sourceLines.size() + 1;
							sourceLines.push_back("#line " + lexical_cast<string>(scanner.getLineNumber()) + ' ' +
																		lexical_cast<string>(depth) + " // " + lines[scanner.getLineNumber()-1]);
							// stop play
						}
						//
						// fragShaderBegins
						//
						else if(token->getCode() == Scanner::TC_IDENTIFIER &&
										strcmp(token->getValue().getString(), "fragShaderBegins") == 0)
						{
							// play

							// its defined in same place so there is probable circular includance
							if(fragShaderBegins.definedInLine==scanner.getLineNumber() && fragShaderBegins.definedInFile==filename)
							{
								PARSE_ERR("fragShaderBegins" << MULTIPLE_DEF_MSG);
								return false;
							}

							if(fragShaderBegins.definedInLine != -1) // if already defined elsewhere throw error
							{
								PARSE_ERR("fragShaderBegins already defined at " << fragShaderBegins.definedInFile << ":" <<
													 fragShaderBegins.definedInLine);
								return false;
							}

							// vert shader entry point not defined
							if(vertShaderBegins.definedInLine == -1)
							{
								PARSE_ERR("fragShaderBegins must follow vertShaderBegins");
								return false;
							}

							fragShaderBegins.definedInFile = filename;
							fragShaderBegins.definedInLine = scanner.getLineNumber();
							fragShaderBegins.globalLine = sourceLines.size() + 1;
							sourceLines.push_back("#line " + lexical_cast<string>(scanner.getLineNumber()) + ' ' +
																		lexical_cast<string>(depth) + " // " + lines[scanner.getLineNumber()-1]);
							// stop play
						}
						//
						// include
						//
						else if(token->getCode() == Scanner::TC_IDENTIFIER && strcmp(token->getValue().getString(), "include") == 0)
						{
							token = &scanner.getNextToken();
							if(token->getCode() == Scanner::TC_STRING)
							{
								// play
								//int line = sourceLines.size();
								sourceLines.push_back("#line 0 " + lexical_cast<string>(depth+1) + " // " +
																			lines[scanner.getLineNumber()-1]);
								if(!parseFileForPragmas(token->getValue().getString(), depth+1))
									return false;
								sourceLines.push_back("#line " + lexical_cast<string>(scanner.getLineNumber()) + ' ' +
																			lexical_cast<string>(depth) +  " // end of " + lines[scanner.getLineNumber()-1]);
								// stop play
							}
							else
							{
								PARSE_ERR_EXPECTED("string");
								return false;
							}
						}
						//
						// transformFeedbackVarying
						//
						else if(token->getCode() == Scanner::TC_IDENTIFIER &&
										strcmp(token->getValue().getString(), "transformFeedbackVarying") == 0)
						{
							token = &scanner.getNextToken();
							if(token->getCode() == Scanner::TC_IDENTIFIER)
							{
								string varName = token->getValue().getString();
								// check if already defined and for circular includance
								Vec<TrffbVaryingPragma>::const_iterator var = findNamed(output.trffbVaryings, varName);
								if(var != output.trffbVaryings.end())
								{
									if(var->definedInLine==scanner.getLineNumber() && var->definedInFile==filename)
									{
										PARSE_ERR("\"" << varName << "\"" << MULTIPLE_DEF_MSG);
									}
									else
									{
										PARSE_ERR("Varying \"" << varName << "\" already defined at " << var->definedInFile << ":" <<
															var->definedInLine);
									}
									return false;
								}

								// all ok, push it back
								output.trffbVaryings.push_back(TrffbVaryingPragma(filename, scanner.getLineNumber(), varName));
								sourceLines.push_back(lines[scanner.getLineNumber()-1]);
							}
							else
							{
								PARSE_ERR_EXPECTED("identifier");
								return false;
							}
						}
						//
						// attribute
						//
						else if(token->getCode() == Scanner::TC_IDENTIFIER && strcmp(token->getValue().getString(), "attribute") == 0)
						{
							token = &scanner.getNextToken();
							if(token->getCode() == Scanner::TC_IDENTIFIER)
							{
								string varName = token->getValue().getString();
								token = &scanner.getNextToken();
								if(token->getCode() == Scanner::TC_NUMBER && token->getDataType() == Scanner::DT_INT)
								{
									uint loc = token->getValue().getInt();

									// check if already defined and for circular includance
									Vec<ShaderVarPragma>::const_iterator attrib = findNamed(output.attributes, varName);
									if(attrib != output.attributes.end())
									{
										if(attrib->definedInLine==scanner.getLineNumber() && attrib->definedInFile==filename)
										{
											PARSE_ERR("\"" << varName << "\"" << MULTIPLE_DEF_MSG);
										}
										else
										{
											PARSE_ERR("Attribute \"" << varName << "\" already defined at " << attrib->definedInFile << ":" <<
																attrib->definedInLine);
										}
										return false;
									}
									// search if another var has the same loc
									for(attrib = output.attributes.begin(); attrib!=output.attributes.end(); ++attrib)
									{
										if(attrib->customLoc == loc)
										{
											PARSE_ERR("The attributes \"" << attrib->name << "\" (" << attrib->definedInFile << ":" <<
																attrib->definedInLine << ") and \"" << varName << "\" share the same location");
											return false;
										}
									}

									// all ok, push it back
									output.attributes.push_back(ShaderVarPragma(filename, scanner.getLineNumber(), varName, loc));
									sourceLines.push_back(lines[scanner.getLineNumber()-1]);
								}
								else
								{
									PARSE_ERR_EXPECTED("integer");
									return false;
								}
							}
							else
							{
								PARSE_ERR_EXPECTED("identifier");
								return false;
							}
						}
						else
						{
							PARSE_ERR("#pragma anki followed by incorrect " << token->getInfoStr());
							return false;
						}
					} // end if anki

					token = &scanner.getNextToken();
					if(token->getCode()!=Scanner::TC_NEWLINE && token->getCode()!=Scanner::TC_EOF)
					{
						PARSE_ERR_EXPECTED("newline or end of file");
						return false;
					}

					if(token->getCode() == Scanner::TC_EOF)
						break;

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
				return false;
			}
		} // end while
	}
	catch(Exception& e)
	{
		ERROR(e.what());
		return false;
	}
	
	return true;
}


//======================================================================================================================
// parseFile                                                                                                           =
//======================================================================================================================
bool ShaderPrePreprocessor::parseFile(const char* filename)
{
	// parse master file
	if(!parseFileForPragmas(filename))
	{
		return false;
	}

	// rearrange the trffbVaryings

	// sanity checks
	if(vertShaderBegins.globalLine == -1)
	{
		ERROR("Entry point \"vertShaderBegins\" is not defined in file \"" << filename << "\"");
		return false;
	}
	if(fragShaderBegins.globalLine == -1)
	{
		ERROR("Entry point \"fragShaderBegins\" is not defined in file \"" << filename << "\"");
		return false;
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
				output.geomShaderSource += sourceLines[i] + "\n";

			output.fragShaderSource += sourceLines[i] + "\n";
		}

		// vert shader code
		int limit = (geomShaderBegins.definedInLine != -1) ? geomShaderBegins.globalLine-1 : fragShaderBegins.globalLine-1;
		for(int i=vertShaderBegins.globalLine-1; i<limit; ++i)
		{
			output.vertShaderSource += sourceLines[i] + "\n";
		}

		// geom shader code
		if(geomShaderBegins.definedInLine != -1)
		{
			for(int i=geomShaderBegins.globalLine-1; i<fragShaderBegins.globalLine-1; ++i)
			{
				output.geomShaderSource += sourceLines[i] + "\n";
			}
		}

		// frag shader code
		for(int i=fragShaderBegins.globalLine-1; i<int(sourceLines.size()); ++i)
		{
			output.fragShaderSource += sourceLines[i] + "\n";
		}
	} // end construct shader's source code

	//PRINT("vertShaderBegins.globalLine: " << vertShaderBegins.globalLine)
	//PRINT("fragShaderBegins.globalLine: " << fragShaderBegins.globalLine)
	//printSourceLines();
	//printShaderVars();

	return true;
}

