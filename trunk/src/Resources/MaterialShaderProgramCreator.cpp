#include "MaterialShaderProgramCreator.h"
#include "Util/scanner/Scanner.h"
#include "Misc/Parser.h"
#include "MaterialBuildinVariable.h"
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>


//==============================================================================
// Statics                                                                     =
//==============================================================================

boost::unordered_map<GLenum, const char*>
MaterialShaderProgramCreator::glTypeToTxt = boost::assign::map_list_of
	(GL_FLOAT, "float")
	(GL_FLOAT_VEC2, "vec2")
	(GL_FLOAT_VEC3, "vec3")
	(GL_FLOAT_VEC4, "vec4")
	(GL_SAMPLER_2D, "sampler2D")
	(GL_FLOAT_MAT3, "mat3")
	(GL_FLOAT_MAT4, "mat4")
	(GL_NONE, "void");


ConstCharPtrHashMap<GLenum>::Type MaterialShaderProgramCreator::txtToGlType =
	boost::assign::map_list_of
	("float", GL_FLOAT)
	("vec2", GL_FLOAT_VEC2)
	("vec3", GL_FLOAT_VEC3)
	("vec4", GL_FLOAT_VEC4)
	("sampler2D", GL_SAMPLER_2D)
	("mat3", GL_FLOAT_MAT3)
	("mat4", GL_FLOAT_MAT4)
	("void", GL_NONE);


boost::unordered_map<MaterialShaderProgramCreator::ArgQualifier, const char*>
	MaterialShaderProgramCreator::argQualifierToTxt = boost::assign::map_list_of
	(AQ_IN, "in")
	(AQ_OUT, "out")
	(AQ_INOUT, "inout");


ConstCharPtrHashMap<MaterialShaderProgramCreator::ArgQualifier>::Type
	MaterialShaderProgramCreator::txtToArgQualifier = boost::assign::map_list_of
	("in", AQ_IN)
	("out", AQ_OUT)
	("inout", AQ_INOUT);


ConstCharPtrHashMap<GLenum>::Type
	MaterialShaderProgramCreator::varyingNameToGlType =
	boost::assign::map_list_of
	("vTexCoords", GL_FLOAT_VEC2)
	("vNormal", GL_FLOAT_VEC3)
	("vTangent", GL_FLOAT_VEC3)
	("vTangentW", GL_FLOAT)
	("vVertPosViewSpace", GL_FLOAT_VEC3);


//==============================================================================
// Constructor                                                                 =
//==============================================================================
MaterialShaderProgramCreator::MaterialShaderProgramCreator(
	const boost::property_tree::ptree& pt)
{
	parseShaderProgramTag(pt);
}


//==============================================================================
// Destructor                                                                  =
//==============================================================================
MaterialShaderProgramCreator::~MaterialShaderProgramCreator()
{}


//==============================================================================
// parseShaderFileForFunctionDefinitions                                       =
//==============================================================================
void MaterialShaderProgramCreator::parseShaderFileForFunctionDefinitions(
	const char* filename)
{
	scanner::Scanner scanner(filename, false);
	const scanner::Token* token = &scanner.getCrntToken();

	// Forever
	while(true)
	{
		getNextTokenAndSkipNewlines(scanner);
		FuncDefinition* funcDef = NULL;

		// EOF
		if(token->getCode() == scanner::TC_END)
		{
			break;
		}
		// #
		else if(token->getCode() == scanner::TC_SHARP)
		{
			parseUntilNewline(scanner);
			continue;
		}
		// Return type
		else if(token->getCode() == scanner::TC_IDENTIFIER)
		{
			funcDef = new FuncDefinition;
			funcDefs.push_back(funcDef);

			try
			{
				funcDef->returnDataType =
					txtToGlType.at(token->getValue().getString());
			}
			catch(std::exception& e)
			{
				throw PARSER_EXCEPTION("Unsupported type: " +
					token->getValue().getString());
			}

			funcDef->returnDataTypeTxt = token->getValue().getString();
		}
		else
		{
			throw PARSER_EXCEPTION_EXPECTED("identifier");
		}

		// Function name
		getNextTokenAndSkipNewlines(scanner);
		if(token->getCode() != scanner::TC_IDENTIFIER)
		{
			throw PARSER_EXCEPTION_EXPECTED("identifier");
		}

		funcDef->name = token->getValue().getString();

		funcNameToDef[funcDef->name.c_str()] = funcDef;

		// (
		getNextTokenAndSkipNewlines(scanner);
		if(token->getCode() != scanner::TC_L_PAREN)
		{
			throw PARSER_EXCEPTION_EXPECTED("(");
		}

		// Arguments
		while(true)
		{
			ArgDefinition* argDef = new ArgDefinition;
			funcDef->argDefinitions.push_back(argDef);

			// Argument qualifier
			getNextTokenAndSkipNewlines(scanner);
			if(token->getCode() != scanner::TC_IDENTIFIER)
			{
				throw PARSER_EXCEPTION_EXPECTED("identifier");
			}

			// For now accept only "in"
			if(strcmp(token->getValue().getString(), "in"))
			{
				throw PARSER_EXCEPTION("Incorrect qualifier: " +
					token->getValue().getString());
			}

			try
			{
				argDef->argQualifier = txtToArgQualifier.at(
					token->getValue().getString());
			}
			catch(std::exception& e)
			{
				throw PARSER_EXCEPTION("Unsupported qualifier: " +
					token->getValue().getString());
			}

			argDef->argQualifierTxt = token->getValue().getString();

			// Type
			getNextTokenAndSkipNewlines(scanner);
			if(token->getCode() != scanner::TC_IDENTIFIER)
			{
				throw PARSER_EXCEPTION_EXPECTED("identifier");
			}

			try
			{
				argDef->dataType = txtToGlType.at(
					token->getValue().getString());
			}
			catch(std::exception& e)
			{
				throw PARSER_EXCEPTION("Unsupported type: " +
					token->getValue().getString());
			}

			argDef->dataTypeTxt = token->getValue().getString();

			// Name
			getNextTokenAndSkipNewlines(scanner);
			if(token->getCode() != scanner::TC_IDENTIFIER)
			{
				throw PARSER_EXCEPTION_EXPECTED("identifier");
			}

			argDef->name = token->getValue().getString();

			// ,
			getNextTokenAndSkipNewlines(scanner);
			if(token->getCode() == scanner::TC_COMMA)
			{
				continue;
			}
			// )
			else if(token->getCode() == scanner::TC_R_PAREN)
			{
				break;
			}
			else
			{
				throw PARSER_EXCEPTION_UNEXPECTED();
			}
		} // End arguments


		// {
		getNextTokenAndSkipNewlines(scanner);
		if(token->getCode() != scanner::TC_L_BRACKET)
		{
			throw PARSER_EXCEPTION_EXPECTED("{");
		}

		// Skip until closing bracket
		int bracketsNum = 0;
		while(true)
		{
			getNextTokenAndSkipNewlines(scanner);

			if(token->getCode() == scanner::TC_R_BRACKET)
			{
				if(bracketsNum == 0)
				{
					break;
				}
				--bracketsNum;
			}
			else if(token->getCode() == scanner::TC_L_BRACKET)
			{
				++bracketsNum;
			}
			else if(token->getCode() == scanner::TC_END)
			{
				throw PARSER_EXCEPTION_UNEXPECTED();
			}
			else
			{
				continue;
			}
		} // End until closing bracket

		/*std::stringstream ss;
		ss << "Func def: " << funcDef->returnArg << " " << funcDef->name << " ";
		BOOST_FOREACH(const ArgDefinition& adef, funcDef->argDefinitions)
		{
			ss << adef.callType << " " << adef.dataType << " " <<
				adef.name << ", ";
		}
		std::cout << ss.str() << std::endl;*/
	} // End for all functions
}


//==============================================================================
// parseUntilNewline                                                           =
//==============================================================================
void MaterialShaderProgramCreator::parseUntilNewline(scanner::Scanner& scanner)
{
	const scanner::Token* token = &scanner.getCrntToken();
	scanner::TokenCode prevTc;

	while(true)
	{
		prevTc = token->getCode();
		scanner.getNextToken();

		if(token->getCode() == scanner::TC_END)
		{
			break;
		}
		else if(token->getCode() == scanner::TC_NEWLINE &&
			prevTc != scanner::TC_BACK_SLASH)
		{
			break;
		}
	}
}


//==============================================================================
// getNextTokenAndSkipNewlines                                                 =
//==============================================================================
void MaterialShaderProgramCreator::getNextTokenAndSkipNewlines(
	scanner::Scanner& scanner)
{
	const scanner::Token* token;

	while(true)
	{
		token = &scanner.getNextToken();
		if(token->getCode() != scanner::TC_NEWLINE)
		{
			break;
		}
	}
}


//==============================================================================
// parseShaderProgramTag                                                       =
//==============================================================================
void MaterialShaderProgramCreator::parseShaderProgramTag(
	const boost::property_tree::ptree& pt)
{
	usingTexCoordsAttrib = usingNormalAttrib = usingTangentAttrib = false;

	using namespace boost::property_tree;

	srcLines.push_back(
		"#pragma anki start vertexShader\n"
		"#pragma anki include \"shaders/MaterialVertex.glsl\"\n"
		"#pragma anki start fragmentShader\n"
		"#pragma anki include \"shaders/MaterialFragmentVariables.glsl\"");

	//
	// <includes></includes>
	//
	boost::ptr_vector<FuncDefinition> funcDefs;
	Vec<std::string> includeLines;

	const ptree& includesPt = pt.get_child("includes");
	BOOST_FOREACH(const ptree::value_type& v, includesPt)
	{
		if(v.first != "include")
		{
			throw EXCEPTION("Expected include and not: " + v.first);
		}

		const std::string& fname = v.second.data();
		parseShaderFileForFunctionDefinitions(fname.c_str());

		includeLines.push_back("#pragma anki include \"" + fname + "\"");
	}

	std::sort(includeLines.begin(), includeLines.end(), compareStrings);
	srcLines.insert(srcLines.end(), includeLines.begin(), includeLines.end());

	//
	// <ins></ins>
	//
	Vec<std::string> uniformsLines; // Store the source of the uniform vars

	boost::optional<const ptree&> insPt = pt.get_child_optional("inputs");
	if(insPt)
	{
		BOOST_FOREACH(const ptree::value_type& v, insPt.get())
		{
			if(v.first != "input")
			{
				throw EXCEPTION("Expected in and not: " + v.first);
			}

			const ptree& inPt = v.second;
			std::string line;
			parseInputTag(inPt, line);
			uniformsLines.push_back(line);
		} // end for all ins

		srcLines.push_back("");
		std::sort(uniformsLines.begin(), uniformsLines.end(), compareStrings);
		srcLines.insert(srcLines.end(), uniformsLines.begin(),
			uniformsLines.end());
	}

	//
	// <operators></operators>
	//
	srcLines.push_back("\nvoid main()\n{");

	const ptree& opsPt = pt.get_child("operators");

	BOOST_FOREACH(const ptree::value_type& v, opsPt)
	{
		if(v.first != "operator")
		{
			throw EXCEPTION("Expected operator and not: " + v.first);
		}

		const ptree& opPt = v.second;
		parseOperatorTag(opPt);
	} // end for all operators

	srcLines.push_back("}\n");

	//
	// Create the output
	//
	if(usingTexCoordsAttrib)
	{
		srcLines.insert(srcLines.begin(), "#define USING_TEX_COORDS_ATTRIB");
	}

	if(usingNormalAttrib)
	{
		srcLines.insert(srcLines.begin(), "#define USING_NORMAL_ATTRIB");
	}

	if(usingTangentAttrib)
	{
		srcLines.insert(srcLines.begin(), "#define USING_TANGENT_ATTRIB");
	}

	BOOST_FOREACH(const std::string& line, srcLines)
	{
		source += line + "\n";
	}

	//std::cout << source << std::endl;
}


//==============================================================================
// parseInputTag                                                               =
//==============================================================================
void MaterialShaderProgramCreator::parseInputTag(
	const boost::property_tree::ptree& pt, std::string& line)
{
	using namespace boost::property_tree;

	const std::string& name = pt.get<std::string>("name");
	boost::optional<const ptree&> valuePt = pt.get_child_optional("value");
	GLenum glType;

	// Buildin or varying
	if(!valuePt)
	{
		MaterialBuildinVariable::MatchingVariable tmp;

		if(MaterialBuildinVariable::isBuildin(name.c_str(), &tmp, &glType))
		{
			const char* glTypeTxt = glTypeToTxt.at(glType);
			line += "uniform ";
			line += glTypeTxt;
		}
		else if(varyingNameToGlType.find(name.c_str()) !=
			varyingNameToGlType.end())
		{
			glType = varyingNameToGlType.at(name.c_str());
			const char* glTypeTxt = glTypeToTxt.at(glType);

			line += "in ";
			line += glTypeTxt;

			// Set a few flags
			if(name == "vTexCoords")
			{
				usingTexCoordsAttrib = true;
			}
			else if(name == "vNormal")
			{
				usingNormalAttrib = true;
			}
			else if(name == "vTangent" || name == "vTangentW")
			{
				usingTangentAttrib = true;
			}
		}
		else
		{
			throw EXCEPTION("The variable is not build-in or varying: " + name);
		}
	}
	else
	{
		if(valuePt.get().size() != 1)
		{
			throw EXCEPTION("Bad value for in: " + name);
		}

		const ptree::value_type& v = valuePt.get().front();

		// Wrong tag
		if(txtToGlType.find(v.first.c_str()) == txtToGlType.end())
		{
			throw EXCEPTION("Wrong type \"" + v.first + "\" for in: " +
				name);
		}

		const std::string& typeTxt = v.first;

		line += "uniform " + typeTxt;
		glType = txtToGlType.at(typeTxt.c_str());
	}

	line += " " + name + ";";
}


//==============================================================================
// parseOperatorTag                                                            =
//==============================================================================
void MaterialShaderProgramCreator::parseOperatorTag(
	const boost::property_tree::ptree& pt)
{
	using namespace boost::property_tree;

	int id = pt.get<int>("id");
	const std::string& funcName = pt.get<std::string>("function");
	const ptree& argsPt = pt.get_child("arguments");
	std::stringstream line;

	// Find func def
	const FuncDefinition* def = NULL;
	try
	{
		def = funcNameToDef.at(funcName.c_str());
	}
	catch(std::exception& e)
	{
		throw EXCEPTION("Function is not defined in any include file: " +
			funcName);
	}

	// Check args size
	if(argsPt.size() != def->argDefinitions.size())
	{
		throw EXCEPTION("Incorrect number of arguments for: " + funcName);
	}

	// Write return value
	if(def->returnDataType != GL_NONE)
	{
		line << def->returnDataTypeTxt << " operatorOut" << id << " = ";
	}

	// Func name
	line << funcName + "(";

	// Write all arguments
	size_t i = 0;
	BOOST_FOREACH(const ptree::value_type& v, argsPt)
	{
		if(v.first != "argument")
		{
			throw EXCEPTION("Unexpected tag \"" + v.first +
				"\" for operator: " + boost::lexical_cast<std::string>(id));

		}

		const std::string& argName = v.second.data();
		line << argName;

		// Add a comma
		++i;
		if(i != argsPt.size())
		{
			line << ", ";
		}
	}
	line << ");";

	srcLines.push_back("\t" + line.str());
}


//==============================================================================
// compareStrings                                                              =
//==============================================================================
bool MaterialShaderProgramCreator::compareStrings(
	const std::string& a, const std::string& b)
{
	return a < b;
}
