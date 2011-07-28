#include "Material2.h"
#include "MaterialVariable.h"
#include "Misc/PropertyTree.h"
#include "MaterialShaderProgramCreator.h"
#include "Core/App.h"
#include "Core/Globals.h"
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/functional/hash.hpp>


//==============================================================================
// Statics                                                                     =
//==============================================================================

// Dont make idiotic mistakes
#define TXT_AND_ENUM(x) \
	(#x, x)


ConstCharPtrHashMap<GLenum>::Type Material2::txtToBlengGlEnum =
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

	const std::string& cpSrc = mspc.getShaderProgramSource();
	std::string dpSrc = "#define DEPTH_PASS\n" + cpSrc;

	createShaderProgSourceToCache(cpSrc);
	createShaderProgSourceToCache(dpSrc);
}


//==============================================================================
// createShaderProgSourceToCache                                               =
//==============================================================================
std::string Material2::createShaderProgSourceToCache(const std::string& source)
{
	// Create the hash
	boost::hash<std::string> stringHash;
	std::size_t h = stringHash(source);
	std::string prefix = boost::lexical_cast<std::string>(h);

	// Create path XXX
	/*boost::filesystem::path newfPathName =
		AppSingleton::getInstance().getCachePath() / (prefix + ".glsl");*/
	boost::filesystem::path newfPathName =
		boost::filesystem::path("/users/panoscc/.anki/cache") / (prefix + ".glsl");


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
	}

	return newfPathName.string();
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

