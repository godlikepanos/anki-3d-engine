#ifndef S_PROG_VAR_H
#define S_PROG_VAR_H

#include <GL/glew.h>
#include <string>
#include "Properties.h"


class ShaderProg;


/// Shader program variable. The type is attribute or uniform
class SProgVar
{
	public:
		/// Shader var types
		enum ShaderVarType
		{
			SVT_ATTRIBUTE, ///< SVT_ATTRIBUTE
			SVT_UNIFORM    ///< SVT_UNIFORM
		};

	PROPERTY_R(GLint, loc, getLoc) ///< GL location
	PROPERTY_R(std::string, name, getName) ///< The name inside the shader program
	/// GL_FLOAT, GL_FLOAT_VEC2 etc. See http://www.opengl.org/sdk/docs/man/xhtml/glGetActiveUniform.xml
	PROPERTY_R(GLenum, glDataType, getGlDataType)
	PROPERTY_R(ShaderVarType, type, getType) ///< @ref SVT_ATTRIBUTE or @ref SVT_UNIFORM

	public:
		SProgVar(GLint loc_, const char* name_, GLenum glDataType_, ShaderVarType type_, const ShaderProg* fatherSProg_);
		SProgVar(const SProgVar& var);
		const ShaderProg& getFatherSProg() const {return *fatherSProg;}

	private:
		const ShaderProg* fatherSProg; ///< We need the ShaderProg of this variable mainly for sanity checks
};


inline SProgVar::SProgVar(GLint loc_, const char* name_, GLenum glDataType_, ShaderVarType type_,
                          const ShaderProg* fatherSProg_):
	loc(loc_),
	name(name_),
	glDataType(glDataType_),
	type(type_),
	fatherSProg(fatherSProg_)
{}


inline SProgVar::SProgVar(const SProgVar& var):
	loc(var.loc),
	name(var.name),
	glDataType(var.glDataType),
	type(var.type),
	fatherSProg(var.fatherSProg)
{}


#endif
