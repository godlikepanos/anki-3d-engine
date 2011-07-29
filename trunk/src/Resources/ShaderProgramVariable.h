#ifndef SHADER_PROGRAM_VARIABLE_H
#define SHADER_PROGRAM_VARIABLE_H

#include <GL/glew.h>
#include <string>
#include <boost/noncopyable.hpp>
#include "Util/Accessors.h"


class ShaderProgram;


/// Shader program variable. The type is attribute or uniform
class ShaderProgramVariable: public boost::noncopyable
{
	public:
		/// Shader var types
		enum ShaderVarType
		{
			SVT_ATTRIBUTE, ///< SVT_ATTRIBUTE
			SVT_UNIFORM    ///< SVT_UNIFORM
		};

		ShaderProgramVariable(GLint loc_, const char* name_,
			GLenum glDataType_, ShaderVarType type_,
			const ShaderProgram& fatherSProg_);

		/// @name Accessors
		/// @{
		const ShaderProgram& getFatherSProg() const {return fatherSProg;}
		GETTER_R(GLint, loc, getLoc)
		GETTER_R(std::string, name, getName)
		GETTER_R(GLenum, glDataType, getGlDataType)
		GETTER_R(ShaderVarType, type, getType)
		/// @}

	private:
		GLint loc; ///< GL location
		std::string name; ///< The name inside the shader program
		/// GL_FLOAT, GL_FLOAT_VEC2 etc. See
		/// http://www.opengl.org/sdk/docs/man/xhtml/glGetActiveUniform.xml
		GLenum glDataType;
		ShaderVarType type; ///< @ref SVT_ATTRIBUTE or @ref SVT_UNIFORM
		/// We need the ShaderProg of this variable mainly for sanity checks
		const ShaderProgram& fatherSProg;
};


inline ShaderProgramVariable::ShaderProgramVariable(GLint loc_,
	const char* name_, GLenum glDataType_,
	ShaderVarType type_, const ShaderProgram& fatherSProg_)
:	loc(loc_),
	name(name_),
	glDataType(glDataType_),
	type(type_),
	fatherSProg(fatherSProg_)
{}


#endif
