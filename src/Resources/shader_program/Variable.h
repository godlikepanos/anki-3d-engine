#ifndef SHADER_PROGRAM_VARIABLE_H
#define SHADER_PROGRAM_VARIABLE_H

#include "Util/Accessors.h"
#include <GL/glew.h>
#include <string>
#include <boost/noncopyable.hpp>
#include "Fwd.h"


namespace shader_program {


/// Shader program variable. The type is attribute or uniform
class Variable: public boost::noncopyable
{
	public:
		/// Shader var types
		enum Type
		{
			ATTRIBUTE,
			UNIFORM
		};

		Variable(GLint loc, const char* name,
			GLenum glDataType, Type type,
			const ShaderProgram& fatherSProg);

		/// @name Accessors
		/// @{

		/// XXX: refactor
		const ShaderProgram& getFatherSProg() const {return fatherSProg;}
		GETTER_R(GLint, loc, getLoc)
		GETTER_R(std::string, name, getName)
		GETTER_R(GLenum, glDataType, getGlDataType)
		GETTER_R(Type, type, getType)
		/// @}

	private:
		GLint loc; ///< GL location
		std::string name; ///< The name inside the shader program
		/// GL_FLOAT, GL_FLOAT_VEC2 etc. See
		/// http://www.opengl.org/sdk/docs/man/xhtml/glGetActiveUniform.xml
		GLenum glDataType;
		Type type; ///< @ref ATTRIBUTE or @ref UNIFORM
		/// We need the ShaderProg of this variable mainly for sanity checks
		const ShaderProgram& fatherSProg;
};


inline Variable::Variable(GLint loc_,
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
