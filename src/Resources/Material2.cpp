#include "Material2.h"
#include "MaterialVariable.h"
#include "Util/Scanner/Scanner.h"
#include "Misc/Parser.h"
#include "Misc/PropertyTree.h"
#include <sstream>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>


//==============================================================================
// Statics                                                                     =
//==============================================================================

Material2::BlendParam Material2::blendingParams[] =
{
	{GL_ZERO, "GL_ZERO"},
	{GL_ONE, "GL_ONE"},
	{GL_DST_COLOR, "GL_DST_COLOR"},
	{GL_ONE_MINUS_DST_COLOR, "GL_ONE_MINUS_DST_COLOR"},
	{GL_SRC_ALPHA, "GL_SRC_ALPHA"},
	{GL_ONE_MINUS_SRC_ALPHA, "GL_ONE_MINUS_SRC_ALPHA"},
	{GL_DST_ALPHA, "GL_DST_ALPHA"},
	{GL_ONE_MINUS_DST_ALPHA, "GL_ONE_MINUS_DST_ALPHA"},
	{GL_SRC_ALPHA_SATURATE, "GL_SRC_ALPHA_SATURATE"},
	{GL_SRC_COLOR, "GL_SRC_COLOR"},
	{GL_ONE_MINUS_SRC_COLOR, "GL_ONE_MINUS_SRC_COLOR"},
	{0, NULL}
};


//==============================================================================
// Constructor                                                                 =
//==============================================================================
Material2::Material2()
{
	renderInBlendingStageFlag = false;
	blendingSfactor = GL_ONE;
	blendingDfactor = GL_ZERO;
	depthTesting = true;
	wireframe = false;
	castsShadowFlag = true;
}


//==============================================================================
// Destructor                                                                  =
//==============================================================================
Material2::~Material2()
{}


//==============================================================================
// parseShaderFileForFunctionDefinitions                                       =
//==============================================================================
void Material2::parseShaderFileForFunctionDefinitions(const char* filename,
	boost::ptr_vector<FuncDefinition>& out)
{
	Scanner::Scanner scanner(filename, false);
	const Scanner::Token* token = &scanner.getCrntToken();

	// Forever
	while(true)
	{
		getNextTokenAndSkipNewline(scanner);
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
		// Type
		else if(token->getCode() == Scanner::TC_IDENTIFIER)
		{
			funcDef = new FuncDefinition;
			out.push_back(funcDef);

			if(!strcmp(token->getValue().getString(), "float"))
			{
				funcDef->returnArg = AT_FLOAT;
			}
			else if(!strcmp(token->getValue().getString(), "vec2"))
			{
				funcDef->returnArg = AT_VEC2;
			}
			else if(!strcmp(token->getValue().getString(), "vec3"))
			{
				funcDef->returnArg = AT_VEC3;
			}
			else if(!strcmp(token->getValue().getString(), "vec4"))
			{
				funcDef->returnArg = AT_VEC4;
			}
			else if(!strcmp(token->getValue().getString(), "void"))
			{
				funcDef->returnArg = AT_VOID;
			}
			else
			{
				throw PARSER_EXCEPTION_EXPECTED("float or vec2 or vec3"
					" or vec4 or void");
			}
		}
		else
		{
			throw PARSER_EXCEPTION_EXPECTED("identifier");
		}

		// Function name
		getNextTokenAndSkipNewline(scanner);
		if(token->getCode() != Scanner::TC_IDENTIFIER)
		{
			throw PARSER_EXCEPTION_EXPECTED("identifier");
		}

		funcDef->name = token->getValue().getString();

		// (
		getNextTokenAndSkipNewline(scanner);
		if(token->getCode() != Scanner::TC_LPAREN)
		{
			throw PARSER_EXCEPTION_EXPECTED("(");
		}

		// Arguments
		while(true)
		{
			ArgDefinition* argDef = new ArgDefinition;
			funcDef->argDefinitions.push_back(argDef);

			// Call type
			getNextTokenAndSkipNewline(scanner);
			if(token->getCode() != Scanner::TC_IDENTIFIER)
			{
				throw PARSER_EXCEPTION_EXPECTED("identifier");
			}

			if(!strcmp(token->getValue().getString(), "in"))
			{
				argDef->callType = ACT_IN;
			}
			else if(!strcmp(token->getValue().getString(), "out"))
			{
				argDef->callType = ACT_OUT;
			}
			else if(!strcmp(token->getValue().getString(), "inout"))
			{
				argDef->callType = ACT_INOUT;
			}
			else
			{
				throw PARSER_EXCEPTION_EXPECTED("in or out or inout");
			}

			// Type
			getNextTokenAndSkipNewline(scanner);
			if(token->getCode() != Scanner::TC_IDENTIFIER)
			{
				throw PARSER_EXCEPTION_EXPECTED("identifier");
			}

			if(!strcmp(token->getValue().getString(), "float"))
			{
				argDef->dataType = AT_FLOAT;
			}
			else if(!strcmp(token->getValue().getString(), "vec2"))
			{
				argDef->dataType = AT_VEC2;
			}
			else if(!strcmp(token->getValue().getString(), "vec3"))
			{
				argDef->dataType = AT_VEC3;
			}
			else if(!strcmp(token->getValue().getString(), "vec4"))
			{
				argDef->dataType = AT_VEC4;
			}
			else if(!strcmp(token->getValue().getString(), "sampler2D"))
			{
				argDef->dataType = AT_TEXTURE;
			}
			else
			{
				throw PARSER_EXCEPTION_EXPECTED("float or vec2 or vec3 or"
					" vec4 or sampler2D");
			}

			// Name
			getNextTokenAndSkipNewline(scanner);
			if(token->getCode() != Scanner::TC_IDENTIFIER)
			{
				throw PARSER_EXCEPTION_EXPECTED("identifier");
			}

			argDef->name = token->getValue().getString();

			// ,
			getNextTokenAndSkipNewline(scanner);
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
		getNextTokenAndSkipNewline(scanner);
		if(token->getCode() != Scanner::TC_LBRACKET)
		{
			throw PARSER_EXCEPTION_EXPECTED("{");
		}

		// Skip until closing bracket
		int bracketsNum = 0;
		while(true)
		{
			getNextTokenAndSkipNewline(scanner);

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
void Material2::parseUntilNewline(Scanner::Scanner& scanner)
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
// getNextTokenAndSkipNewline                                                  =
//==============================================================================
void Material2::getNextTokenAndSkipNewline(Scanner::Scanner& scanner)
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
// getGlBlendEnumFromText                                                      =
//==============================================================================
int Material2::getGlBlendEnumFromText(const char* str)
{
	BlendParam* ptr = &blendingParams[0];
	while(true)
	{
		if(ptr->str == NULL)
		{
			throw EXCEPTION("Incorrect blending factor: " + str);
		}

		if(!strcmp(ptr->str, str))
		{
			return ptr->glEnum;
		}

		++ptr;
	}
}


//==============================================================================
// parseMaterialTag                                                            =
//==============================================================================
void Material2::parseMaterialTag(const boost::property_tree::ptree& pt)
{
	using namespace boost::property_tree;

	//
	// castsShadow
	//
	boost::optional<bool> shadow =
		PropertyTree::getBoolOptional(pt, "castsShadow");
	if(shadow)
	{
		castsShadowFlag = shadow.get();
	}

	//
	// renderInBlendingStage
	//
	boost::optional<bool> bs =
		PropertyTree::getBoolOptional(pt, "renderInBlendingStage");
	if(bs)
	{
		renderInBlendingStageFlag = bs.get();
	}

	//
	// blendFunctions
	//
	boost::optional<const ptree&> blendFuncsTree =
		pt.get_child_optional("blendFunctions");
	if(blendFuncsTree)
	{
		// sFactor
		blendingSfactor = getGlBlendEnumFromText(
			blendFuncsTree.get().get<std::string>("sFactor").c_str());

		// dFactor
		blendingDfactor = getGlBlendEnumFromText(
			blendFuncsTree.get().get<std::string>("dFactor").c_str());
	}

	//
	// depthTesting
	//
	boost::optional<bool> dp =
		PropertyTree::getBoolOptional(pt, "depthTesting");
	if(dp)
	{
		depthTesting = dp.get();
	}

	//
	// wireframe
	//
	boost::optional<bool> wf =
		PropertyTree::getBoolOptional(pt, "wireframe");
	if(wf)
	{
		wireframe = wf.get();
	}

	//
	// shaderProgram
	//
	parseShaderProgramTag(pt.get_child("shaderProgram"));
}


//==============================================================================
// parseShaderProgramTag                                                       =
//==============================================================================
void Material2::parseShaderProgramTag(const boost::property_tree::ptree& pt)
{
	using namespace boost::property_tree;

	Vec<std::string> srcLines;

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
		//parseShaderFileForFunctionDefinitions(fname.c_str(), funcDefs);

		includeLines.push_back("#pragma anki include \"" + fname + "\"");
	}

	std::sort(includeLines.begin(), includeLines.end(), compareStrings);
	srcLines.assign(includeLines.begin(), includeLines.end());

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

			const std::string& name = inPt.get<std::string>("name");

			std::string line;

			if(inPt.get_child_optional("float"))
			{
				line += "float";
			}
			else if(inPt.get_child_optional("vec2"))
			{
				line += "vec2";
			}
			else if(inPt.get_child_optional("vec3"))
			{
			}
			else if(inPt.get_child_optional("vec4"))
			{
			}
			else if(inPt.get_child_optional("sampler2D"))
			{
			}
			else
			{
				BuildinMaterialVariable::BuildinVariable tmp;
				GLenum dataType;

				if(BuildinMaterialVariable::isBuildin(name.c_str(),
					&tmp, &dataType))
				{
					throw EXCEPTION("The variable is not build in: " + name);
				}
			}
		}
	}

	//
	//
	//
	std::string src;
	BOOST_FOREACH(const std::string& line, srcLines)
	{
		src += line + "\n";
	}

	std::cout << src << std::endl;
}


//==============================================================================
// load                                                                        =
//==============================================================================
void Material2::load(const char* filename)
{
	try
	{
		using namespace boost::property_tree;
		ptree pt;
		read_xml(filename, pt);
		parseMaterialTag(pt.get_child("material"));
	}
	catch(std::exception& e)
	{
		throw EXCEPTION("File \"" + filename + "\" failed: " + e.what());
	}
}


//==============================================================================
// compareStrings                                                              =
//==============================================================================
bool Material2::compareStrings(const std::string& a, const std::string& b)
{
	return a < b;
}
