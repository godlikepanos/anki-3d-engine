#include "Material2.h"
#include "Util/Scanner/Scanner.h"
#include "Misc/Parser.h"


//==============================================================================
// Statics                                                                     =
//==============================================================================

boost::array<Material2::StdVarNameAndGlDataTypePair, Material2::SAV_NUM>
	Material2::stdAttribVarInfos =
{{
	{"position", GL_FLOAT_VEC3},
	{"tangent", GL_FLOAT_VEC4},
	{"normal", GL_FLOAT_VEC3},
	{"texCoords", GL_FLOAT_VEC2}
}};


boost::array<Material2::StdVarNameAndGlDataTypePair, Material2::SUV_NUM>
	Material2::stdUniVarInfos =
{{
	{"modelMat", GL_FLOAT_MAT4},
	{"viewMat", GL_FLOAT_MAT4},
	{"projectionMat", GL_FLOAT_MAT4},
	{"modelViewMat", GL_FLOAT_MAT4},
	{"ViewProjectionMat", GL_FLOAT_MAT4},
	{"normalMat", GL_FLOAT_MAT3},
	{"modelViewProjectionMat", GL_FLOAT_MAT4},
	{"msNormalFai", GL_SAMPLER_2D},
	{"msDiffuseFai", GL_SAMPLER_2D},
	{"msSpecularFai", GL_SAMPLER_2D},
	{"msDepthFai", GL_SAMPLER_2D},
	{"isFai", GL_SAMPLER_2D},
	{"ppsPrePassFai", GL_SAMPLER_2D},
	{"ppsPostPassFai", GL_SAMPLER_2D},
	{"rendererSize", GL_FLOAT_VEC2},
	{"sceneAmbientColor", GL_FLOAT_VEC3},
	{"blurring", GL_FLOAT},
}};


//==============================================================================
// parseShaderFileForFunctionDefinitions                                       =
//==============================================================================
void Material2::parseShaderFileForFunctionDefinitions(const char* filename,
	boost::ptr_vector<FuncDefinition>& out)
{
	Scanner::Scanner scanner(filename);
	const Scanner::Token* token;

	while(true)
	{
		// EOF
		if(token->getCode() == Scanner::TC_EOF)
		{
			break;
		}

		FuncDefinition* funcDef = new FuncDefinition;
		out.push_back(funcDef);

		// Type
		token = &scanner.getNextToken();
		if(token->getCode() == Scanner::TC_IDENTIFIER)
		{
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
		scanner.getNextToken();
		if(token->getCode() != Scanner::TC_IDENTIFIER)
		{
			throw PARSER_EXCEPTION_EXPECTED("identifier");
		}

		funcDef->name = token->getValue().getString();

		// (
		scanner.getNextToken();
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
			scanner.getNextToken();
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
			scanner.getNextToken();
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
			scanner.getNextToken();
			if(token->getCode() != Scanner::TC_IDENTIFIER)
			{
				throw PARSER_EXCEPTION_EXPECTED("identifier");
			}

			argDef->name = token->getValue().getString();

			// ,
			scanner.getNextToken();
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
		scanner.getNextToken();
		if(token->getCode() != Scanner::TC_LBRACKET)
		{
			throw PARSER_EXCEPTION_EXPECTED("{");
		}

		// Skip until closing bracket
		int bracketsNum = 0;
		while(true)
		{
			scanner.getNextToken();

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
			else
			{
				continue;
			}
		} // End until closing bracket
	} // End for all functions
}
