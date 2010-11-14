#include <cstring>
#include "Material.h"
#include "Resource.h"
#include "Scanner.h"
#include "Parser.h"
#include "Texture.h"
#include "ShaderProg.h"
#include "App.h"
#include "MainRenderer.h"


/// Customized THROW_EXCEPTION
#define MTL_EXCEPTION(x) EXCEPTION("Material \"" + getRsrcPath() + getRsrcName() + "\": " + x)


//======================================================================================================================
// Statics                                                                                                             =
//======================================================================================================================
Material::StdVarNameAndGlDataTypePair Material::stdAttribVarInfos[SAV_NUM] =
{
	{"position", GL_FLOAT_VEC3},
	{"tangent", GL_FLOAT_VEC4},
	{"normal", GL_FLOAT_VEC3},
	{"texCoords", GL_FLOAT_VEC2},
	{"vertWeightBonesNum", GL_FLOAT},
	{"vertWeightBoneIds", GL_FLOAT_VEC4},
	{"vertWeightWeights", GL_FLOAT_VEC4}
};

Material::StdVarNameAndGlDataTypePair Material::stdUniVarInfos[SUV_NUM] =
{
	{"skinningRotations", GL_FLOAT_MAT3},
	{"skinningTranslations", GL_FLOAT_VEC3},
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
	{"sceneAmbientColor", GL_FLOAT_VEC3}
};

Material::PreprocDefines Material::msGenericDefines [] =
{
	{"DIFFUSE_MAPPING", 'd'},
	{"NORMAL_MAPPING", 'n'},
	{"SPECULAR_MAPPING", 's'},
	{"PARALLAX_MAPPING", 'p'},
	{"ENVIRONMENT_MAPPING", 'e'},
	{"ALPHA_TESTING", 'a'},
	{"HARDWARE_SKINNING", 'h'},
	{NULL, NULL}
};

Material::PreprocDefines Material::dpGenericDefines [] =
{
	{"ALPHA_TESTING", 'a'},
	{"HARDWARE_SKINNING", 'h'},
	{NULL, NULL}
};


//======================================================================================================================
// Blending stuff                                                                                                      =
//======================================================================================================================

struct BlendParam
{
	int glEnum;
	const char* str;
};

static BlendParam blendingParams[] =
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

static bool searchBlendEnum(const char* str, int& gl_enum)
{
	BlendParam* ptr = &blendingParams[0];
	while(true)
	{
		if(ptr->str == NULL)
		{
			break;
		}

		if(!strcmp(ptr->str, str))
		{
			gl_enum = ptr->glEnum;
			return true;
		}
	}

	return false;
}


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
Material::Material():
	Resource(RT_MATERIAL)
{
	blends = false;
	blendingSfactor = GL_ONE;
	blendingDfactor = GL_ZERO;
	depthTesting = true;
	wireframe = false;
	castsShadow = true;
}


//======================================================================================================================
// load                                                                                                                =
//======================================================================================================================
void Material::load(const char* filename)
{
	Scanner scanner(filename);
	const Scanner::Token* token;

	while(true)
	{
		token = &scanner.getNextToken();

		//
		// Shader program
		//
		if(Parser::isIdentifier(token, "shaderProg"))
		{
			if(shaderProg.get())
			{
				throw PARSER_EXCEPTION("Shader program already loaded");
			}

			token = &scanner.getNextToken();
			std::string shaderFilename;
			// Just a string
			if(token->getCode() == Scanner::TC_STRING)
			{
				shaderFilename = token->getValue().getString();
			}
			// Its { so... build custom shader
			else if(token->getCode() == Scanner::TC_LBRACKET)
			{
				std::string sProgFilename;
				std::string source;
				std::string prefix;

				std::string op = Parser::parseIdentifier(scanner);
				if(op == "customMsSProg")
				{
					parseCustomShader(msGenericDefines, scanner, sProgFilename, source, prefix);
				}
				else if(op == "customDpSProg")
				{
					parseCustomShader(dpGenericDefines, scanner, sProgFilename, source, prefix);
				}
				else
				{
					throw PARSER_EXCEPTION_EXPECTED("identifier customMsSProg or customDpSProg");
				}

				shaderFilename = ShaderProg::createSrcCodeToCache(sProgFilename.c_str(), source.c_str(), prefix.c_str());

				// }
				token = &scanner.getNextToken();
				if(token->getCode() != Scanner::TC_RBRACKET)
				{
					throw PARSER_EXCEPTION_EXPECTED("}");
				}
			}
			else
			{
				throw PARSER_EXCEPTION_EXPECTED("string or {");
			}

			shaderProg.loadRsrc(shaderFilename.c_str());
		}
		//
		// dpMtl
		//
		else if(Parser::isIdentifier(token, "dpMtl"))
		{
			if(dpMtl.get())
			{
				throw PARSER_EXCEPTION("Depth material already loaded");
			}

			dpMtl.loadRsrc(Parser::parseString(scanner).c_str());
		}
		//
		// blendingStage
		//
		else if(Parser::isIdentifier(token, "blendingStage"))
		{
			blends = Parser::parseBool(scanner);
		}
		//
		// blendFuncs
		//
		else if(Parser::isIdentifier(token, "blendFuncs"))
		{
			// {
			token = &scanner.getNextToken();
			if(token->getCode() != Scanner::TC_LBRACKET)
			{
				throw PARSER_EXCEPTION_EXPECTED("{");
			}

			// sFactor
			Parser::parseIdentifier(scanner, "sFactor");
			int gl_enum;
			if(!searchBlendEnum(Parser::parseIdentifier(scanner).c_str(), gl_enum))
			{
				throw PARSER_EXCEPTION("Incorrect blending factor \"" + token->getValue().getString() + "\"");
			}
			blendingSfactor = gl_enum;

			// dFactor
			Parser::parseIdentifier(scanner, "dFactor");
			if(!searchBlendEnum(Parser::parseIdentifier(scanner).c_str(), gl_enum))
			{
				throw PARSER_EXCEPTION("Incorrect blending factor \"" + token->getValue().getString() + "\"");
			}
			blendingDfactor = gl_enum;

		}
		//
		// depthTesting
		//
		else if(Parser::isIdentifier(token, "depthTesting"))
		{
			depthTesting = Parser::parseBool(scanner);
		}
		//
		// wireframe
		//
		else if(Parser::isIdentifier(token, "wireframe"))
		{
			wireframe = Parser::parseBool(scanner);
		}
		//
		// castsShadow
		//
		else if(Parser::isIdentifier(token, "castsShadow"))
		{
			castsShadow = Parser::parseBool(scanner);
		}
		//
		// userDefinedVars
		//
		else if(Parser::isIdentifier(token, "userDefinedVars"))
		{
			// first check if the shader is defined
			if(shaderProg.get() == NULL)
			{
				throw PARSER_EXCEPTION("You have to define the shader program before the user defined vars");
			}

			// {
			token = &scanner.getNextToken();
			if(token->getCode() != Scanner::TC_LBRACKET)
			{
				throw PARSER_EXCEPTION_EXPECTED("{");
			}
			// loop all the vars
			do
			{
				// read the name
				token = &scanner.getNextToken();
				if(token->getCode() == Scanner::TC_RBRACKET)
				{
					break;
				}

				if(token->getCode() != Scanner::TC_IDENTIFIER)
				{
					throw PARSER_EXCEPTION_EXPECTED("identifier");
				}

				std::string varName;
				varName = token->getValue().getString();

				userDefinedVars.push_back(new UserDefinedUniVar); // create new var
				UserDefinedUniVar& var = userDefinedVars.back();

				// check if the uniform exists
				if(!shaderProg->uniVarExists(varName.c_str()))
				{
					throw PARSER_EXCEPTION("The variable \"" + varName + "\" is not an active uniform");
				}

				var.sProgVar = shaderProg->findUniVar(varName.c_str());

				// read the values
				switch(var.sProgVar->getGlDataType())
				{
					// texture
					case GL_SAMPLER_2D:
						var.value.texture.loadRsrc(Parser::parseString(scanner).c_str());
						break;
					// float
					case GL_FLOAT:
						token = &scanner.getNextToken();
						if(token->getCode() == Scanner::TC_NUMBER && token->getDataType() == Scanner::DT_FLOAT)
						{
							var.value.float_ = token->getValue().getFloat();
						}
						else
						{
							throw PARSER_EXCEPTION_EXPECTED("float");
						}
						break;
					// vec2
					case GL_FLOAT_VEC2:
						Parser::parseMathVector(scanner, var.value.vec2);
						break;
					// vec3
					case GL_FLOAT_VEC3:
						Parser::parseMathVector(scanner, var.value.vec3);
						break;
					// vec4
					case GL_FLOAT_VEC4:
						Parser::parseMathVector(scanner, var.value.vec4);
						break;
				};

			}while(true); // end loop for all the vars

		}
		//
		// EOF
		//
		else if(token->getCode() == Scanner::TC_EOF)
		{
			break;
		}
		//
		// other crap
		//
		else
		{
			throw PARSER_EXCEPTION_UNEXPECTED();
		}
	} // end while

	initStdShaderVars();
}


//======================================================================================================================
// initStdShaderVars                                                                                                   =
//======================================================================================================================
void Material::initStdShaderVars()
{
	// sanity checks
	if(!shaderProg.get())
	{
		throw MTL_EXCEPTION("Without shader is like cake without sugar (missing SHADER_PROG)");
	}

	// the attributes
	for(uint i=0; i<SAV_NUM; i++)
	{
		// if the var is not in the sProg then... bye
		if(!shaderProg->attribVarExists(stdAttribVarInfos[i].varName))
		{
			stdAttribVars[i] = NULL;
			continue;
		}

		// set the std var
		stdAttribVars[i] = shaderProg->findAttribVar(stdAttribVarInfos[i].varName);

		// check if the shader has different GL data type from that it suppose to have
		if(stdAttribVars[i]->getGlDataType() != stdAttribVarInfos[i].dataType)
		{
			throw MTL_EXCEPTION("The \"" + stdAttribVarInfos[i].varName +
			                    "\" attribute var has incorrect GL data type from the expected");
		}
	}

	// the uniforms
	for(uint i=0; i<SUV_NUM; i++)
	{
		// if the var is not in the sProg then... bye
		if(!shaderProg->uniVarExists(stdUniVarInfos[i].varName))
		{
			stdUniVars[i] = NULL;
			continue;
		}

		// set the std var
		stdUniVars[i] = shaderProg->findUniVar(stdUniVarInfos[i].varName);

		// check if the shader has different GL data type from that it suppose to have
		if(stdUniVars[i]->getGlDataType() != stdUniVarInfos[i].dataType)
		{
			throw MTL_EXCEPTION("The \"" + stdUniVarInfos[i].varName +
			                    "\" uniform var has incorrect GL data type from the expected");
		}
	}
}


//======================================================================================================================
// parseCustomShader                                                                                                   =
//======================================================================================================================
void Material::parseCustomShader(const PreprocDefines defines[], Scanner& scanner,
		                             std::string& shaderFilename, std::string& source, std::string& prefix)
{
	const Scanner::Token* token;

	// {
	token = &scanner.getNextToken();
	if(token->getCode() != Scanner::TC_LBRACKET)
	{
		throw PARSER_EXCEPTION_EXPECTED("{");
	}

	// file
	Parser::parseIdentifier(scanner, "file");

	// the shader prog
	shaderFilename = Parser::parseString(scanner);

	// defines
	Parser::parseIdentifier(scanner, "defines");

	// {
	token = &scanner.getNextToken();
	if(token->getCode() != Scanner::TC_LBRACKET)
	{
		throw PARSER_EXCEPTION_EXPECTED("{");
	}

	// Get the defines
	while(true)
	{
		token = &scanner.getNextToken();

		// }
		if(token->getCode() == Scanner::TC_RBRACKET)
		{
			break;
		}
		else if(token->getCode() != Scanner::TC_IDENTIFIER)
		{
			throw PARSER_EXCEPTION_EXPECTED("identifier");
		}

		// Check if acceptable value. Loop the switches array
		const PreprocDefines* def = defines;
		while(def->switchName != NULL)
		{
			if(!strcmp(def->switchName, token->getString()))
			{
				break;
			}

			++def;
		}

		if(def->switchName == NULL)
		{
			throw PARSER_EXCEPTION("Not acceptable define " + token->getString());
		}

		source += std::string("#define ") + token->getString() + "\n";
		prefix.push_back(def->prefix);
	} // end get the switches

	// }
	token = &scanner.getNextToken();
	if(token->getCode() != Scanner::TC_RBRACKET)
	{
		throw PARSER_EXCEPTION_EXPECTED("}");
	}

	std::sort(prefix.begin(), prefix.end());
}
