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
#define MTL_THROW_EXCEPTION(x) THROW_EXCEPTION("Material \"" + getRsrcPath() + getRsrcName() + "\": " + x)


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


//======================================================================================================================
// Stuff for custom material stage shader prgs                                                                         =
//======================================================================================================================

/// A simple pair-like structure
struct MsSwitch
{
	const char* switchName;
	const char prefix;
};


/// See the docs for info about the switches
static MsSwitch msSwitches [] =
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
	{GL_ONE_MINUS_SRC_COLOR, "GL_ONE_MINUS_SRC_COLOR"}
};

const int BLEND_PARAMS_NUM = 11;

static bool searchBlendEnum(const char* str, int& gl_enum)
{
	for(int i=0; i<BLEND_PARAMS_NUM; i++)
	{
		if(!strcmp(blendingParams[i].str, str))
		{
			gl_enum = blendingParams[i].glEnum;
			return true;
		}
	}
	return false;
}


//======================================================================================================================
// load                                                                                                                =
//======================================================================================================================
void Material::load(const char* filename)
{
	try
	{
		Scanner scanner(filename);
		const Scanner::Token* token;

		while(true)
		{
			token = &scanner.getNextToken();

			//
			// SHADER_PROG
			//
			if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "SHADER_PROG"))
			{
				if(shaderProg.get())
				{
					PARSER_THROW_EXCEPTION("Shader program already loaded");
				}

				token = &scanner.getNextToken();
				string shaderFilename;
				// Just a string
				if(token->getCode() == Scanner::TC_STRING)
				{
					shaderFilename = token->getValue().getString();
				}
				// build custom shader
				else if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getString(), "buildMsSProg"))
				{
					// (
					token = &scanner.getNextToken();
					if(token->getCode() != Scanner::TC_LPAREN)
					{
						PARSER_THROW_EXCEPTION_EXPECTED("(");
					}

					// shader prog
					token = &scanner.getNextToken();
					if(token->getCode() != Scanner::TC_STRING)
					{
						PARSER_THROW_EXCEPTION_EXPECTED("string");
					}
					string sProgFilename = token->getValue().getString();

					// get the switches
					string source;
					string prefix;
					while(true)
					{
						token = &scanner.getNextToken();

						if(token->getCode() == Scanner::TC_RPAREN)
						{
							break;
						}

						if(token->getCode() != Scanner::TC_IDENTIFIER)
						{
							PARSER_THROW_EXCEPTION_EXPECTED("identifier");
						}

						// Check if acceptable value. Loop the switches array
						MsSwitch* mss = msSwitches;
						while(mss->switchName != NULL)
						{
							if(!strcmp(mss->switchName, token->getString()))
							{
								break;
							}

							++mss;
						}

						if(mss->switchName == NULL)
						{
							PARSER_THROW_EXCEPTION("Incorrect switch " + token->getString());
						}

						source += string("#define ") + token->getString() + "\n";
						prefix.push_back(mss->prefix);
					} // end get the switches

					std::sort(prefix.begin(), prefix.end());

					shaderFilename = ShaderProg::createSrcCodeToCache(sProgFilename.c_str(), source.c_str(), prefix.c_str());
				}
				else
				{
					PARSER_THROW_EXCEPTION_EXPECTED("string or buildMsSProg");
				}

				shaderProg.loadRsrc(shaderFilename.c_str());
			}
			//
			// DEPTH_PASS_MATERIAL
			//
			else if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "DEPTH_PASS_MATERIAL"))
			{
				if(dpMtl.get())
				{
					PARSER_THROW_EXCEPTION("Depth material already loaded");
				}

				token = &scanner.getNextToken();
				if(token->getCode() != Scanner::TC_STRING)
				{
					PARSER_THROW_EXCEPTION_EXPECTED("string");
				}
				dpMtl.loadRsrc(token->getValue().getString());
			}
			//
			// BLENDS
			//
			else if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "BLENDS"))
			{
				token = &scanner.getNextToken();
				if(token->getCode() != Scanner::TC_NUMBER)
				{
					PARSER_THROW_EXCEPTION_EXPECTED("number");
				}
				blends = token->getValue().getInt();
			}
			//
			// BLENDING_SFACTOR
			//
			else if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "BLENDING_SFACTOR"))
			{
				token = &scanner.getNextToken();
				if(token->getCode() != Scanner::TC_IDENTIFIER)
				{
					PARSER_THROW_EXCEPTION_EXPECTED("identifier");
				}
				int gl_enum;
				if(!searchBlendEnum(token->getValue().getString(), gl_enum))
				{
					PARSER_THROW_EXCEPTION("Incorrect blending factor \"" + token->getValue().getString() + "\"");
				}
				blendingSfactor = gl_enum;
			}
			//
			// BLENDING_DFACTOR
			//
			else if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "BLENDING_DFACTOR"))
			{
				token = &scanner.getNextToken();
				if(token->getCode() != Scanner::TC_IDENTIFIER)
				{
					PARSER_THROW_EXCEPTION_EXPECTED("identifier");
				}
				int gl_enum;
				if(!searchBlendEnum(token->getValue().getString(), gl_enum))
				{
					PARSE_ERR("Incorrect blending factor \"" << token->getValue().getString() << "\"");
				}
				blendingDfactor = gl_enum;
			}
			//
			// DEPTH_TESTING
			//
			else if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "DEPTH_TESTING"))
			{
				token = &scanner.getNextToken();
				if(token->getCode() != Scanner::TC_NUMBER)
				{
					PARSER_THROW_EXCEPTION_EXPECTED("number");
				}
				depthTesting = token->getValue().getInt();
			}
			//
			// WIREFRAME
			//
			else if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "WIREFRAME"))
			{
				token = &scanner.getNextToken();
				if(token->getCode() != Scanner::TC_NUMBER)
				{
					PARSER_THROW_EXCEPTION_EXPECTED("number");
				}
				wireframe = token->getValue().getInt();
			}
			//
			// CASTS_SHADOW
			//
			else if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "CASTS_SHADOW"))
			{
				token = &scanner.getNextToken();
				if(token->getCode() != Scanner::TC_NUMBER)
				{
					PARSER_THROW_EXCEPTION_EXPECTED("number");
				}
				castsShadow = token->getValue().getInt();
			}
			//
			// USER_DEFINED_VARS
			//
			else if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "USER_DEFINED_VARS"))
			{
				// first check if the shader is defined
				if(shaderProg.get() == NULL)
				{
					PARSER_THROW_EXCEPTION("You have to define the shader program before the user defined vars");
				}

				// {
				token = &scanner.getNextToken();
				if(token->getCode() != Scanner::TC_LBRACKET)
				{
					PARSER_THROW_EXCEPTION_EXPECTED("{");
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
						PARSER_THROW_EXCEPTION_EXPECTED("identifier");
					}

					string varName;
					varName = token->getValue().getString();

					userDefinedVars.push_back(new UserDefinedUniVar); // create new var
					UserDefinedUniVar& var = userDefinedVars.back();

					// check if the uniform exists
					if(!shaderProg->uniVarExists(varName.c_str()))
					{
						PARSER_THROW_EXCEPTION("The variable \"" + varName + "\" is not an active uniform");
					}

					var.sProgVar = shaderProg->findUniVar(varName.c_str());

					// read the values
					switch(var.sProgVar->getGlDataType())
					{
						// texture
						case GL_SAMPLER_2D:
							token = &scanner.getNextToken();
							if(token->getCode() == Scanner::TC_STRING)
							{
								var.value.texture.loadRsrc(token->getValue().getString());
							}
							else
							{
								PARSER_THROW_EXCEPTION_EXPECTED("string");
							}
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
								PARSER_THROW_EXCEPTION_EXPECTED("float");
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
				PARSER_THROW_EXCEPTION_UNEXPECTED();
			}
		} // end while
	}
	catch(std::exception& e)
	{
		ERROR(e.what());
	}

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
		MTL_THROW_EXCEPTION("Without shader is like cake without sugar (missing SHADER_PROG)");
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
			MTL_THROW_EXCEPTION("The \"" + stdAttribVarInfos[i].varName +
			                    "\" attribute var has incorrect GL data type from the expected (0x" + hex +
			                    stdAttribVars[i]->getGlDataType() + ")");
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
			MTL_THROW_EXCEPTION("The \"" + stdUniVarInfos[i].varName +
			                    "\" uniform var has incorrect GL data type from the expected (0x" + hex +
			                    stdUniVars[i]->getGlDataType() + ")");
		}
	}
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

