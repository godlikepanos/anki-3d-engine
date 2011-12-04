#ifndef ANKI_RESOURCE_SHADER_PROGRAM_VARIABLE_H
#define ANKI_RESOURCE_SHADER_PROGRAM_VARIABLE_H

#include <GL/glew.h>
#include <string>
#include <boost/noncopyable.hpp>


namespace anki {


class ShaderProgram;


/// Shader program variable. The type is attribute or uniform
class ShaderProgramVariable: public boost::noncopyable
{
public:
	/// Shader var types
	enum Type
	{
		T_ATTRIBUTE,
		T_UNIFORM
	};

	ShaderProgramVariable(GLint loc, const char* name,
		GLenum glDataType, Type type,
		const ShaderProgram& fatherSProg);

	virtual ~ShaderProgramVariable()
	{}

	/// @name Accessors
	/// @{
	const ShaderProgram& getFatherSProg() const
	{
		return fatherSProg;
	}

	GLint getLocation() const
	{
		return loc;
	}

	const std::string& getName() const
	{
		return name;
	}

	GLenum getGlDataType() const
	{
		return glDataType;
	}

	Type getType() const
	{
		return type;
	}
	/// @}

private:
	GLint loc; ///< GL location
	std::string name; ///< The name inside the shader program
	/// GL_FLOAT, GL_FLOAT_VEC2 etc. See
	/// http://www.opengl.org/sdk/docs/man/xhtml/glGetActiveUniform.xml
	GLenum glDataType;
	Type type; ///< @ref T_ATTRIBUTE or @ref T_UNIFORM
	/// We need the ShaderProg of this variable mainly for sanity checks
	const ShaderProgram& fatherSProg;
};


inline ShaderProgramVariable::ShaderProgramVariable(GLint loc_,
	const char* name_, GLenum glDataType_,
	Type type_, const ShaderProgram& fatherSHADER_PROGRAM_)
:	loc(loc_),
	name(name_),
	glDataType(glDataType_),
	type(type_),
	fatherSProg(fatherSHADER_PROGRAM_)
{}


} // end namespace


#endif
