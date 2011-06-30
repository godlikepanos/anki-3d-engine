#ifndef PROPERTY_TREE_H
#define PROPERTY_TREE_H

#include <boost/optional.hpp>
#include <boost/property_tree/ptree_fwd.hpp>
#include "Math/Math.h"


namespace PropertyTree {

/// Get a bool from a ptree or throw exception if not found or incorrect.
/// Get something like this: @code<tag>true</tag>@endcode
/// @param[in] pt The ptree
/// @param[in] tag The name of tha tag
extern bool getBool(const boost::property_tree::ptree& pt, const char* tag);

/// Optionaly get a bool from a ptree and throw exception incorrect.
/// Get something like this: @code<tag>true</tag>@endcode
/// @param[in] pt The ptree
/// @param[in] tag The name of tha tag
extern boost::optional<bool> getBoolOptional(const boost::property_tree::ptree& pt, const char* tag);

/// Get a @code<float>0.0</float>@endcode
extern float getFloat(const boost::property_tree::ptree& pt);

/// Get a @code<vec2><x>0.0</x><y>0.0</y></vec2>@endcode
extern Vec2 getVec2(const boost::property_tree::ptree& pt);

/// Get a @code<vec3><x>0.0</x><y>0.0</y><z>0.0</z></vec3>@endcode
extern Vec3 getVec3(const boost::property_tree::ptree& pt);

/// Get a @code<vec4><x>0.0</x><y>0.0</y><z>0.0</z><w>0.0</w></vec4>@endcode
extern Vec4 getVec4(const boost::property_tree::ptree& pt);

} // end namespace

#endif
