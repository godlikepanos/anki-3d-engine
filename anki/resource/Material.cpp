#include "anki/resource/Material.h"
#include "anki/resource/MaterialVariable.h"
#include "anki/misc/PropertyTree.h"
#include "anki/resource/MaterialShaderProgramCreator.h"
#include "anki/core/App.h"
#include "anki/core/Globals.h"
#include "anki/resource/ShaderProgram.h"
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/functional/hash.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <algorithm>


namespace anki {


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
Material::Material()
{}


//==============================================================================
Material::~Material()
{}


//==============================================================================
void Material::load(const char* filename)
{
	fname = filename;
	try
	{
		using namespace boost::property_tree;
		ptree pt;
		read_xml(filename, pt);
		parseMaterialTag(pt.get_child("material"));
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("File \"" + filename + "\" failed: " + e.what());
	}
}


//==============================================================================
void Material::parseMaterialTag(const boost::property_tree::ptree& pt)
{
	using namespace boost::property_tree;

	//
	// renderingStage
	//
	renderingStage = pt.get<int>("renderingStage");

	//
	// passes
	//
	boost::optional<std::string> pass =
		pt.get_optional<std::string>("passes");

	if(pass)
	{
		passes = splitString(pass.get().c_str());
	}
	else
	{
		passes.push_back("DUMMY");
	}

	//
	// levelsOfDetail
	//
	boost::optional<int> lod = pt.get_optional<int>("levelsOfDetail");

	if(lod)
	{
		levelsOfDetail = lod.get();
	}
	else
	{
		levelsOfDetail = 1;
	}

	//
	// shadow
	//
	boost::optional<int> sw = pt.get_optional<int>("shadow");

	if(sw)
	{
		shadow = sw.get();
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
				throw ANKI_EXCEPTION("Incorrect blend enum: " + tmp);
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
				throw ANKI_EXCEPTION("Incorrect blend enum: " + tmp);
			}

			blendingDfactor = it->second;
		}
	}

	//
	// depthTesting
	//
	boost::optional<int> dp = pt.get_optional<int>("depthTesting");

	if(dp)
	{
		depthTesting = dp.get();
	}

	//
	// wireframe
	//
	boost::optional<int> wf = pt.get_optional<int>("wireframe");

	if(wf)
	{
		wireframe = wf.get();
	}

	//
	// shaderProgram
	//
	MaterialShaderProgramCreator mspc(pt.get_child("shaderProgram"));

	for(uint level = 0; level < levelsOfDetail; ++level)
	{
		for(uint pid = 0; pid < passes.size(); ++pid)
		{
			std::stringstream src;

			src << "#define LEVEL_" << level << std::endl;
			src << "#define PASS_" << passes[pid] << std::endl;
			src << mspc.getShaderProgramSource() << std::endl;

			std::string filename =
				createShaderProgSourceToCache(src.str().c_str());

			ShaderProgramResourcePointer* pptr =
				new ShaderProgramResourcePointer;

			pptr->load(filename.c_str());

			ShaderProgram* sprog = pptr->get();

			sProgs.push_back(pptr);

			eSProgs[PassLevelKey(pid, level)] = sprog;
		}
	}

	populateVariables(pt.get_child("shaderProgram.inputs"));
}


//==============================================================================
std::string Material::createShaderProgSourceToCache(const std::string& source)
{
	// Create the hash
	boost::hash<std::string> stringHash;
	std::size_t h = stringHash(source);
	std::string prefix = boost::lexical_cast<std::string>(h);

	// Create path
	boost::filesystem::path newfPathName =
		AppSingleton::get().getCachePath() / (prefix + ".glsl");


	// If file not exists write it
	if(!boost::filesystem::exists(newfPathName))
	{
		// If not create it
		std::ofstream f(newfPathName.string().c_str());
		if(!f.is_open())
		{
			throw ANKI_EXCEPTION("Cannot open file for writing: " +
				newfPathName.string());
		}

		f.write(source.c_str(), source.length());
		f.close();
	}

	return newfPathName.string();
}


//==============================================================================
void Material::populateVariables(const boost::property_tree::ptree& pt)
{
	using namespace boost::property_tree;

	//
	// Get all names of all shader prog vars. Dont duplicate
	//
	std::map<std::string, GLenum> allVarNames;

	BOOST_FOREACH(const ShaderProgramResourcePointer& sProg, sProgs)
	{
		BOOST_FOREACH(const ShaderProgramVariable& v, sProg->getVariables())
		{
			allVarNames[v.getName()] = v.getGlDataType();
		}
	}

	//
	// Iterate shader program variables
	//
	std::map<std::string, GLenum>::const_iterator it = allVarNames.begin();
	for(; it != allVarNames.end(); it++)
	{
		const char* svName = it->first.c_str();
		GLenum dataType = it->second;

		// Find the ptree that contains the value
		const ptree* valuePt = NULL;
		BOOST_FOREACH(const ptree::value_type& v, pt)
		{
			if(v.first != "input")
			{
				throw ANKI_EXCEPTION("Expecting <input> and not: " +
					v.first);
			}

			if(v.second.get<std::string>("name") == svName)
			{
				valuePt = &v.second.get_child("value");
				break;
			}
		}

		MaterialVariable* v = NULL;

		// Buildin?
		if(!valuePt)
		{
			v = new MaterialVariable(svName, eSProgs);
		}
		// User defined
		else
		{
			const std::string& value = valuePt->get<std::string>("value");

			// Get the value
			switch(dataType)
			{
				// sampler2D
				case GL_SAMPLER_2D:
					v = new MaterialVariable(svName, eSProgs,
						value);
					break;
				// float
				case GL_FLOAT:
					v = new MaterialVariable(svName, eSProgs,
						boost::lexical_cast<float>(value));
					break;
				// vec2
				case GL_FLOAT_VEC2:
					v = new MaterialVariable(svName, eSProgs,
						setMathType<Vec2, 2>(value.c_str()));
					break;
				// vec3
				case GL_FLOAT_VEC3:
					v = new MaterialVariable(svName, eSProgs,
						setMathType<Vec3, 3>(value.c_str()));
					break;
				// vec4
				case GL_FLOAT_VEC4:
					v = new MaterialVariable(svName, eSProgs,
						setMathType<Vec4, 4>(value.c_str()));
					break;
				// default is error
				default:
					ANKI_ASSERT(0);
			}
		}

		vars.push_back(v);
		nameToVar[v->getName().c_str()] = v;
	} // end for all sprog vars
}


//==============================================================================
StringList Material::splitString(const char* str)
{
	StringList out;
	std::string s = str;
	boost::tokenizer<> tok(s);

	for(boost::tokenizer<>::iterator it = tok.begin(); it != tok.end(); ++it)
	{
		out.push_back(*it);
	}

	return out;
}


//==============================================================================
template<typename Type, size_t n>
Type Material::setMathType(const char* str)
{
	Type out;
	StringList sl = splitString(str);
	ANKI_ASSERT(sl.size() == n);

	for(uint i = 0; i < n; ++i)
	{
		out[i] = boost::lexical_cast<float>(sl[i]);
	}

	return out;
}


//==============================================================================
const MaterialVariable& Material::findVariableByName(const char* name) const
{
	NameToVariableHashMap::const_iterator it = nameToVar.find(name);
	if(it == nameToVar.end())
	{
		throw ANKI_EXCEPTION("Cannot get material variable "
			"with name \"" + name + '\"');
	}
	return *(it->second);
}


} // end namespace
