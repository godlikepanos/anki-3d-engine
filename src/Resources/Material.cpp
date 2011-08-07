#include "Material.h"
#include "MaterialVariable.h"
#include "Misc/PropertyTree.h"
#include "MaterialShaderProgramCreator.h"
#include "Core/App.h"
#include "Core/Globals.h"
#include "ShaderProgram.h"
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/functional/hash.hpp>
#include <algorithm>


//==============================================================================
// Statics                                                                     =
//==============================================================================

// Dont make idiotic mistakes
#define TXT_AND_ENUM(x) \
	(#x, x)


ConstCharPtrHashMap<GLenum>::Type Material::txtToBlengGlEnum =
	boost::assign::map_list_of
	TXT_AND_ENUM(GL_ZERO)
	TXT_AND_ENUM(GL_ONE)
	TXT_AND_ENUM(GL_DST_COLOR)
	TXT_AND_ENUM(GL_ONE_MINUS_DST_COLOR)
	TXT_AND_ENUM(GL_SRC_ALPHA)
	TXT_AND_ENUM(GL_ONE_MINUS_SRC_ALPHA)
	TXT_AND_ENUM(GL_DST_ALPHA)
	TXT_AND_ENUM(GL_ONE_MINUS_DST_ALPHA)
	TXT_AND_ENUM(GL_SRC_ALPHA_SATURATE)
	TXT_AND_ENUM(GL_SRC_COLOR)
	TXT_AND_ENUM(GL_ONE_MINUS_SRC_COLOR);


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

	// Reset tha array
	std::fill(buildinsArr.begin(), buildinsArr.end(),
		static_cast<BuildinMaterialVariable*>(NULL));
}


//==============================================================================
// Destructor                                                                  =
//==============================================================================
Material::~Material()
{}


//==============================================================================
// load                                                                        =
//==============================================================================
void Material::load(const char* filename)
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
// parseMaterialTag                                                            =
//==============================================================================
void Material::parseMaterialTag(const boost::property_tree::ptree& pt)
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
		{
			const std::string& tmp =
				blendFuncsTree.get().get<std::string>("sFactor");

			ConstCharPtrHashMap<GLenum>::Type::const_iterator it =
				txtToBlengGlEnum.find(tmp.c_str());

			if(it == txtToBlengGlEnum.end())
			{
				throw EXCEPTION("Incorrect blend enum: " + tmp);
			}

			blendingSfactor = it->second;
		}

		// dFactor
		{
			const std::string& tmp =
				blendFuncsTree.get().get<std::string>("dFactor");

			ConstCharPtrHashMap<GLenum>::Type::const_iterator it =
				txtToBlengGlEnum.find(tmp.c_str());

			if(it == txtToBlengGlEnum.end())
			{
				throw EXCEPTION("Incorrect blend enum: " + tmp);
			}

			blendingDfactor = it->second;
		}
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
	MaterialShaderProgramCreator mspc(pt.get_child("shaderProgram"));

	std::string cpSrc = "#define COLOR_PASS\n" + mspc.getShaderProgramSource();
	std::string dpSrc = "#define DEPTH_PASS\n" + mspc.getShaderProgramSource();

	std::string cfile = createShaderProgSourceToCache(cpSrc);
	std::string dfile = createShaderProgSourceToCache(dpSrc);

	sProgs[MaterialVariable::COLOR_PASS].loadRsrc(cfile.c_str());
	sProgs[MaterialVariable::DEPTH_PASS].loadRsrc(dfile.c_str());

	/*INFO(cpShaderProg->getShaderInfoString());
	INFO(dpShaderProg->getShaderInfoString());*/

	populateVariables(pt.get_child("shaderProgram.inputs"));
}


//==============================================================================
// createShaderProgSourceToCache                                               =
//==============================================================================
std::string Material::createShaderProgSourceToCache(const std::string& source)
{
	// Create the hash
	boost::hash<std::string> stringHash;
	std::size_t h = stringHash(source);
	std::string prefix = boost::lexical_cast<std::string>(h);

	// Create path
	boost::filesystem::path newfPathName =
		AppSingleton::getInstance().getCachePath() / (prefix + ".glsl");


	// If file not exists write it
	if(!boost::filesystem::exists(newfPathName))
	{
		// If not create it
		std::ofstream f(newfPathName.string().c_str());
		if(!f.is_open())
		{
			throw EXCEPTION("Cannot open file for writing: " +
				newfPathName.string());
		}

		f.write(source.c_str(), source.length());
		f.close();
	}

	return newfPathName.string();
}


//==============================================================================
// populateVariables                                                           =
//==============================================================================
void Material::populateVariables(const boost::property_tree::ptree& pt)
{
	using namespace boost::property_tree;

	//
	// Get all names of all shader prog vars. Dont duplicate
	//
	std::map<std::string, GLenum> allVarNames;

	BOOST_FOREACH(const RsrcPtr<ShaderProgram>& sProg, sProgs)
	{
		BOOST_FOREACH(const ShaderProgramVariable& v, sProg->getVariables())
		{
			allVarNames[v.getName()] = v.getGlDataType();
		}
	}

	//
	// Iterate shader program variables
	//
	MaterialVariable::ShaderPrograms sProgs_;
	for(uint i = 0; i < MaterialVariable::PASS_TYPES_NUM; i++)
	{
		sProgs_[i] = sProgs[i].get();
	}

	std::map<std::string, GLenum>::const_iterator it = allVarNames.begin();
	for(; it != allVarNames.end(); it++)
	{
		const char* svName = it->first.c_str();
		GLenum dataType = it->second;

		// Buildin?
		if(BuildinMaterialVariable::isBuildin(svName))
		{
			BuildinMaterialVariable* v =
				new BuildinMaterialVariable(svName, sProgs_);

			mtlVars.push_back(v);
			buildinsArr[v->getVariableEnum()] = v;
		}
		// User defined
		else
		{
			// Find the ptree that contains the value
			const ptree* valuePt = NULL;
			BOOST_FOREACH(const ptree::value_type& v, pt)
			{
				if(v.first != "input")
				{
					throw EXCEPTION("Expecting <input> and not: " + v.first);
				}

				if(v.second.get<std::string>("name") == svName)
				{
					valuePt = &v.second.get_child("value");
					break;
				}
			}

			if(valuePt == NULL)
			{
				throw EXCEPTION("Variable not buildin and not found: " +
					svName);
			}

			UserMaterialVariable* v = NULL;
			// Get the value
			switch(dataType)
			{
				// sampler2D
				case GL_SAMPLER_2D:
					v = new UserMaterialVariable(svName, sProgs_,
						valuePt->get<std::string>("sampler2D").c_str());
					break;
				// float
				case GL_FLOAT:
					v = new UserMaterialVariable(svName, sProgs_,
						PropertyTree::getFloat(*valuePt));
					break;
				// vec2
				case GL_FLOAT_VEC2:
					v = new UserMaterialVariable(svName, sProgs_,
						PropertyTree::getVec2(*valuePt));
					break;
				// vec3
				case GL_FLOAT_VEC3:
					v = new UserMaterialVariable(svName, sProgs_,
						PropertyTree::getVec3(*valuePt));
					break;
				// vec4
				case GL_FLOAT_VEC4:
					v = new UserMaterialVariable(svName, sProgs_,
						PropertyTree::getVec4(*valuePt));
					break;
				// default is error
				default:
					ASSERT(0);
			}

			mtlVars.push_back(v);
			userMtlVars.push_back(v);
		}
	} // end for all sprog vars
}
