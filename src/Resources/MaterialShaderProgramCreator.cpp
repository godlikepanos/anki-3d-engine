#include "MaterialShaderProgramCreator.h"
#include "Util/Scanner/Scanner.h"
#include "Misc/Parser.h"
#include "BuildinMaterialVariable.h"
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
	Scanner::Scanner scanner(filename, false);
	const Scanner::Token* token = &scanner.getCrntToken();

	// Forever
	while(true)
	{
		getNextTokenAndSkipNewlines(scanner);
		FuncDefinition* funcDef = NULL;

		// EOF
		if(token->getCode() == Scanner::TC_EOF)
		{
			break;
		}
		// #
		else if(token->getCode() == Scanner::TC_SHARP)
		{
			parseUntilNewline(scanner);
			continue;
		}
		// Return type
		else if(token->getCode() == Scanner::TC_IDENTIFIER)
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
		if(token->getCode() != Scanner::TC_IDENTIFIER)
		{
			throw PARSER_EXCEPTION_EXPECTED("identifier");
		}

		funcDef->name = token->getValue().getString();

		funcNameToDef[funcDef->name.c_str()] = funcDef;

		// (
		getNextTokenAndSkipNewlines(scanner);
		if(token->getCode() != Scanner::TC_LPAREN)
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
			if(token->getCode() != Scanner::TC_IDENTIFIER)
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
			if(token->getCode() != Scanner::TC_IDENTIFIER)
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
			if(token->getCode() != Scanner::TC_IDENTIFIER)
			{
				throw PARSER_EXCEPTION_EXPECTED("identifier");
			}

			argDef->name = token->getValue().getString();

			// ,
			getNextTokenAndSkipNewlines(scanner);
			if(token->getCode() == Scanner::TC_COMMA)
			{
				continue;
			}
			// )
			else if(token->getCode() == Scanner::TC_RPAREN)
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
		if(token->getCode() != Scanner::TC_LBRACKET)
		{
			throw PARSER_EXCEPTION_EXPECTED("{");
		}

		// Skip until closing bracket
		int bracketsNum = 0;
		while(true)
		{
			getNextTokenAndSkipNewlines(scanner);

			if(token->getCode() == Scanner::TC_RBRACKET)
			{
				if(bracketsNum == 0)
				{
					break;
				}
				--bracketsNum;
			}
			else if(token->getCode() == Scanner::TC_LBRACKET)
			{
				++bracketsNum;
			}
			else if(token->getCode() == Scanner::TC_EOF)
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
void MaterialShaderProgramCreator::parseUntilNewline(Scanner::Scanner& scanner)
{
	const Scanner::Token* token = &scanner.getCrntToken();
	Scanner::TokenCode prevTc;

	while(true)
	{
		prevTc = token->getCode();
		scanner.getNextToken();

		if(token->getCode() == Scanner::TC_EOF)
		{
			break;
		}
		else if(token->getCode() == Scanner::TC_NEWLINE &&
			prevTc != Scanner::TC_BACK_SLASH)
		{
			break;
		}
	}
}


//==============================================================================
// getNextTokenAndSkipNewlines                                                 =
//==============================================================================
void MaterialShaderProgramCreator::getNextTokenAndSkipNewlines(
	Scanner::Scanner& scanner)
{
	const Scanner::Token* token;

	while(true)
	{
		token = &scanner.getNextToken();
		if(token->getCode() != Scanner::TC_NEWLINE)
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
	using namespace boost::property_tree;

	srcLines.push_back("#pragma anki start vertexShader");
	srcLines.push_back("#pragma anki include \"shaders/MaterialVert.glsl\"");
	srcLines.push_back("#pragma anki start fragmentShader");

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

	boost::optional<const ptree&> insPt = pt.get_child_optional("ins");
	if(insPt)
	{
		BOOST_FOREACH(const ptree::value_type& v, insPt.get())
		{
			if(v.first != "in")
			{
				throw EXCEPTION("Expected in and not: " + v.first);
			}

			const ptree& inPt = v.second;
			std::string line;
			parseInTag(inPt, line);
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
	srcLines.push_back("\n#pragma anki include "
		"\"shaders/MaterialFragmentVariables.glsl\"\n"
		"\nvoid main()\n{");

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
	BOOST_FOREACH(const std::string& line, srcLines)
	{
		source += line + "\n";
	}

	//std::cout << source << std::endl;
}


//==============================================================================
// parseInTag                                                                  =
//==============================================================================
void MaterialShaderProgramCreator::parseInTag(
	const boost::property_tree::ptree& pt, std::string& line)
{
	using namespace boost::property_tree;

	const std::string& name = pt.get<std::string>("name");
	boost::optional<const ptree&> valuePt = pt.get_child_optional("value");
	GLenum glType;

	line = "uniform ";

	// Buildin
	if(!valuePt)
	{
		BuildinMaterialVariable::BuildinVariable tmp;

		if(!BuildinMaterialVariable::isBuildin(name.c_str(), &tmp, &glType))
		{
			throw EXCEPTION("The variable is not build in: " + name);
		}

		boost::unordered_map<GLenum, const char*>::const_iterator it =
			glTypeToTxt.find(glType);

		ASSERT(it != glTypeToTxt.end() &&
			"Buildin's type is not registered");

		line += it->second;
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

		line += typeTxt;
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
	ConstCharPtrHashMap<FuncDefinition*>::Type::const_iterator it =
		funcNameToDef.find(funcName.c_str());

	if(it == funcNameToDef.end())
	{
		throw EXCEPTION("Function is not defined in any include file: " +
			funcName);
	}

	const FuncDefinition* def = it->second;

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

		line << v.second.data();

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
