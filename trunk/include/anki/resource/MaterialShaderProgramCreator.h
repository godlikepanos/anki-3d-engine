#ifndef ANKI_RESOURCE_MATERIAL_SHADER_PROGRAM_CREATOR_H
#define ANKI_RESOURCE_MATERIAL_SHADER_PROGRAM_CREATOR_H

#include "anki/util/StringList.h"
#include <boost/property_tree/ptree_fwd.hpp>


namespace anki {


/// Creator of shader programs. This class parses between
/// <shaderProgam></shaderProgram> located inside a <material></material>
/// and creates the source of a custom program.
class MaterialShaderProgramCreator
{
public:
	MaterialShaderProgramCreator(const boost::property_tree::ptree& pt);
	~MaterialShaderProgramCreator();

	/// Get the shader program source code. This is the one and only public
	/// member
	const std::string& getShaderProgramSource() const
	{
		return source;
	}

private:
	/// The lines of the shader program source
	StringList srcLines;

	std::string source; ///< Shader program final source

	/// Used for shorting vectors of strings. Used in std::sort
	static bool compareStrings(const std::string& a, const std::string& b);

	/// Parse what is within the
	/// @code <shaderProgram></shaderProgram> @endcode
	void parseShaderProgramTag(const boost::property_tree::ptree& pt);

	/// Parse what is within the
	/// @code <shader></shader> @endcode
	void parseShaderTag(const boost::property_tree::ptree& pt);

	/// Parse what is within the @code <input></input> @endcode
	void parseInputTag(const boost::property_tree::ptree& pt,
		std::string& line);

	/// Parse what is within the @code <operation></operation> @endcode
	void parseOperationTag(const boost::property_tree::ptree& pt);
};


} // end namespace


#endif
