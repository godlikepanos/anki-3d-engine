#include <boost/property_tree/ptree.hpp>
#include "anki/misc/PropertyTree.h"
#include "anki/util/Exception.h"


namespace anki {
namespace PropertyTree {


//==============================================================================
// getBool                                                                     =
//==============================================================================
bool getBool(const boost::property_tree::ptree& pt, const char* tag)
{
	std::string str = pt.get<std::string>(tag);
	if(str == "true")
	{
		return true;
	}
	else if(str == "false")
	{
		return false;
	}
	else
	{
		throw ANKI_EXCEPTION("Expected true or false for tag " + tag +
			" and not " + str);
	}
}


//==============================================================================
// getBoolOptional                                                             =
//==============================================================================
extern boost::optional<bool> getBoolOptional(
	const boost::property_tree::ptree& pt, const char* tag)
{
	boost::optional<std::string> str = pt.get_optional<std::string>(tag);
	if(str)
	{
		if(str.get() == "true")
		{
			return boost::optional<bool>(true);
		}
		else if(str.get() == "false")
		{
			return boost::optional<bool>(true);
		}
		else
		{
			throw ANKI_EXCEPTION("Expected true or false for tag " + tag +
				" and not " + str.get());
		}
	}
	return boost::optional<bool>();
}


//==============================================================================
// getFloat                                                                    =
//==============================================================================
float getFloat(const boost::property_tree::ptree& pt)
{
	return pt.get<float>("float");
}


//==============================================================================
// getVec2                                                                     =
//==============================================================================
Vec2 getVec2(const boost::property_tree::ptree& pt)
{
	const boost::property_tree::ptree& tree = pt.get_child("vec2");
	return Vec2(tree.get<float>("x"), tree.get<float>("y"));
}


//==============================================================================
// getVec3                                                                     =
//==============================================================================
Vec3 getVec3(const boost::property_tree::ptree& pt)
{
	const boost::property_tree::ptree& tree = pt.get_child("vec3");
	return Vec3(
		tree.get<float>("x"),
		tree.get<float>("y"),
		tree.get<float>("z"));
}


//==============================================================================
// getVec4                                                                     =
//==============================================================================
Vec4 getVec4(const boost::property_tree::ptree& pt)
{
	const boost::property_tree::ptree& tree = pt.get_child("vec4");
	return Vec4(
		tree.get<float>("x"),
		tree.get<float>("y"),
		tree.get<float>("z"),
		tree.get<float>("w"));
}



} // end namespace
} // end namespace
