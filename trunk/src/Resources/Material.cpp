#include <cstring>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include "Resources/Material.h"
#include "Resources/Texture.h"
#include "Resources/ShaderProg.h"
#include "Misc/PropertyTree.h"


//==============================================================================
// Statics                                                                     =
//==============================================================================
boost::array<Material::StdVarNameAndGlDataTypePair, Material::SAV_NUM> Material::stdAttribVarInfos =
{{
	{"position", GL_FLOAT_VEC3},
	{"tangent", GL_FLOAT_VEC4},
	{"normal", GL_FLOAT_VEC3},
	{"texCoords", GL_FLOAT_VEC2}
}};

boost::array<Material::StdVarNameAndGlDataTypePair, Material::SUV_NUM> Material::stdUniVarInfos =
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

Material::PreprocDefines Material::msGenericDefines[] =
{
	{"DIFFUSE_MAPPING", 'd'},
	{"NORMAL_MAPPING", 'n'},
	{"SPECULAR_MAPPING", 's'},
	{"PARALLAX_MAPPING", 'p'},
	{"ENVIRONMENT_MAPPING", 'e'},
	{"ALPHA_TESTING", 'a'},
	{0, 0}
};

Material::PreprocDefines Material::dpGenericDefines[] =
{
	{"ALPHA_TESTING", 'a'},
	{0, 0}
};


//==============================================================================
// Blending stuff                                                              =
//==============================================================================

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

		++ptr;
	}

	return false;
}


//==============================================================================
// Constructor                                                                 =
//==============================================================================
Material::Material()
{
	renderInBlendingStageFlag = false;
	blendingSfactor = GL_ONE;
	blendingDfactor = GL_ZERO;
	depthTesting = true;
	wireframe = false;
	castsShadowFlag = true;
}


//==============================================================================
// load                                                                        =
//==============================================================================
void Material::load(const char* filename)
{
	try
	{
		using namespace boost::property_tree;
		ptree rpt;
		read_xml(filename, rpt);

		const ptree& pt = rpt.get_child("material");

		//
		// shaderProg
		//
		const ptree& shaderProgTree = pt.get_child("shaderProg");

		boost::optional<std::string> file = shaderProgTree.get_optional<std::string>("file");
		boost::optional<const ptree&> customMsSProgTree = shaderProgTree.get_child_optional("customMsSProg");
		boost::optional<const ptree&> customDpSProgTree = shaderProgTree.get_child_optional("customDpSProg");

		// Just file
		if(file)
		{
			shaderProg.loadRsrc(file.get().c_str());
		}
		// customMsSProg
		else if(customMsSProgTree)
		{
			std::string source;

			parseCustomShader(msGenericDefines, customMsSProgTree.get(), source);
			std::string shaderFilename = ShaderProg::createSrcCodeToCache("shaders/MsMpGeneric.glsl",
																		  source.c_str());
			shaderProg.loadRsrc(shaderFilename.c_str());
		}
		// customDpSProg
		else if(customDpSProgTree)
		{
			std::string source;

			parseCustomShader(dpGenericDefines, customDpSProgTree.get(), source);
			std::string shaderFilename = ShaderProg::createSrcCodeToCache("shaders/DpGeneric.glsl",
																		  source.c_str());
			shaderProg.loadRsrc(shaderFilename.c_str());
		}
		// Error
		else
		{
			throw EXCEPTION("Expected file or customMsSProg or customDpSProg");
		}

		//
		// renderInBlendingStageFlag
		//
		boost::optional<bool> blendingStage_ = PropertyTree::getBoolOptional(pt, "blendingStage");
		if(blendingStage_)
		{
			renderInBlendingStageFlag = blendingStage_.get();
		}

		//
		// blendFuncs
		//
		boost::optional<const ptree&> blendFuncsTree = pt.get_child_optional("blendFuncs");
		if(blendFuncsTree)
		{
			int glEnum;

			// sFactor
			std::string sFactor_ = blendFuncsTree.get().get<std::string>("sFactor");
			if(!searchBlendEnum(sFactor_.c_str(), glEnum))
			{
				throw EXCEPTION("Incorrect blending factor \"" + sFactor_ + "\"");
			}
			blendingSfactor = glEnum;

			// dFactor
			std::string dFactor_ = blendFuncsTree.get().get<std::string>("dFactor");
			if(!searchBlendEnum(dFactor_.c_str(), glEnum))
			{
				throw EXCEPTION("Incorrect blending factor \"" + dFactor_ + "\"");
			}
			blendingDfactor = glEnum;
		}

		//
		// depthTesting
		//
		boost::optional<bool> depthTesting_ = PropertyTree::getBoolOptional(pt, "depthTesting");
		if(depthTesting_)
		{
			depthTesting = depthTesting_.get();
		}

		//
		// wireframe
		//
		boost::optional<bool> wireframe_ = PropertyTree::getBoolOptional(pt, "wireframe");
		if(wireframe_)
		{
			wireframe = wireframe_.get();
		}

		//
		// userDefinedVars
		//
		boost::optional<const ptree&> userDefinedVarsTree = pt.get_child_optional("userDefinedVars");
		if(userDefinedVarsTree)
		{
			BOOST_FOREACH(const ptree::value_type& v, userDefinedVarsTree.get())
			{
				if(v.first != "userDefinedVar")
				{
					throw EXCEPTION("Expected userDefinedVar and not " + v.first);
				}

				const ptree& userDefinedVarTree = v.second;
				std::string varName = userDefinedVarTree.get<std::string>("name");

				// check if the uniform exists
				if(!shaderProg->uniVarExists(varName.c_str()))
				{
					throw EXCEPTION("The variable \"" + varName + "\" is not an active uniform");
				}

				const ptree& valueTree = userDefinedVarTree.get_child("value");

				const SProgUniVar& uni = *shaderProg->findUniVar(varName.c_str());

				// read the values
				switch(uni.getGlDataType())
				{
					// texture
					case GL_SAMPLER_2D:
					{
						boost::optional<std::string> texture = valueTree.get_optional<std::string>("texture");
						boost::optional<std::string> fai = valueTree.get_optional<std::string>("fai");

						if(texture)
						{
							userDefinedVars.push_back(new MtlUserDefinedVar(uni, texture.get()));
						}
						else if(fai)
						{
							if(fai.get() == "msDepthFai")
							{
								userDefinedVars.push_back(new MtlUserDefinedVar(uni, MtlUserDefinedVar::MS_DEPTH_FAI));
							}
							else if(fai.get() == "isFai")
							{
								userDefinedVars.push_back(new MtlUserDefinedVar(uni, MtlUserDefinedVar::IS_FAI));
							}
							else if(fai.get() == "ppsPrePassFai")
							{
								userDefinedVars.push_back(new MtlUserDefinedVar(uni,
								                                                MtlUserDefinedVar::PPS_PRE_PASS_FAI));
							}
							else if(fai.get() == "ppsPostPassFai")
							{
								userDefinedVars.push_back(new MtlUserDefinedVar(uni,
								                                                MtlUserDefinedVar::PPS_POST_PASS_FAI));
							}
							else
							{
								throw EXCEPTION("incorrect FAI");
							}
						}
						else
						{
							throw EXCEPTION("texture or fai expected");
						}

						break;
					}
					// float
					case GL_FLOAT:
						userDefinedVars.push_back(new MtlUserDefinedVar(uni, PropertyTree::getFloat(valueTree)));
						break;
					// vec2
					case GL_FLOAT_VEC2:
						userDefinedVars.push_back(new MtlUserDefinedVar(uni, PropertyTree::getVec2(valueTree)));
						break;
					// vec3
					case GL_FLOAT_VEC3:
						userDefinedVars.push_back(new MtlUserDefinedVar(uni, PropertyTree::getVec3(valueTree)));
						break;
					// vec4
					case GL_FLOAT_VEC4:
						userDefinedVars.push_back(new MtlUserDefinedVar(uni, PropertyTree::getVec4(valueTree)));
						break;
				};
			} // end for all userDefinedVars
		} // end userDefinedVars
		initStdShaderVars();
	}
	catch(std::exception& e)
	{
		throw EXCEPTION("Material \"" + filename + "\": " + e.what());
	}
}


//==============================================================================
// initStdShaderVars                                                           =
//==============================================================================
void Material::initStdShaderVars()
{
	// sanity checks
	if(!shaderProg.get())
	{
		throw EXCEPTION("Material without shader program is like cake without sugar");
	}

	// the attributes
	for(uint i = 0; i < SAV_NUM; i++)
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
			throw EXCEPTION("The \"" + stdAttribVarInfos[i].varName +
			                "\" attribute var has incorrect GL data type from the expected");
		}
	}

	// the uniforms
	for(uint i = 0; i < SUV_NUM; i++)
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
			throw EXCEPTION("The \"" + stdUniVarInfos[i].varName +
			                "\" uniform var has incorrect GL data type from the expected");
		}
	}
}


//==============================================================================
// parseCustomShader                                                           =
//==============================================================================
void Material::parseCustomShader(const PreprocDefines defines[], const boost::property_tree::ptree& pt,
                                 std::string& source)
{
	using namespace boost::property_tree;

	boost::optional<const ptree&> definesTree = pt.get_child_optional("defines");
	if(!definesTree)
	{
		return;
	}

	BOOST_FOREACH(const ptree::value_type& v, definesTree.get())
	{
		if(v.first != "define")
		{
			throw EXCEPTION("Found " + v.first + " in defines");
		}

		const std::string& define = v.second.data();

		const PreprocDefines* def = defines;
		while(def->switchName != NULL)
		{
			if (define == def->switchName)
			{
				break;
			}
			++def;
		}

		if(def->switchName == NULL)
		{
			throw EXCEPTION("Not acceptable define " + define);
		}

		source += "#define " + define + "\n";
	}
}
