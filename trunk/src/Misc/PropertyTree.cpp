#include <boost/property_tree/ptree.hpp>
#include "PropertyTree.h"
#include "Exception.h"


namespace PropertyTree {

//======================================================================================================================
// getBool                                                                                                             =
//======================================================================================================================
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
		throw EXCEPTION("Expected true or false for tag " + tag + " and not " + str);
	}
}


//======================================================================================================================
// getBoolOptional                                                                                                     =
//======================================================================================================================
extern boost::optional<bool> getBoolOptional(const boost::property_tree::ptree& pt, const char* tag)
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
			throw EXCEPTION("Expected true or false for tag " + tag + " and not " + str.get());
		}
	}
	return boost::optional<bool>();
}


//======================================================================================================================
// getFloat                                                                                                            =
//======================================================================================================================
float getFloat(const boost::property_tree::ptree& pt)
{
	return pt.get<float>("float");
}


//======================================================================================================================
// getVec3                                                                                                             =
//======================================================================================================================
Vec3 getVec3(const boost::property_tree::ptree& pt)
{
	const boost::property_tree::ptree& vec3Tree = pt.get_child("vec3");
	Vec3 v3;
	v3.x = vec3Tree.get<float>("x");
	v3.y = vec3Tree.get<float>("y");
	v3.z = vec3Tree.get<float>("z");
	return v3;
}



} // end namespace
