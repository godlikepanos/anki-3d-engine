#ifndef MATERIAL_VARIABLE_H
#define MATERIAL_VARIABLE_H

#include "Common.h"
#include "../shader_program/Variable.h"
#include "Util/Accessors.h"
#include "Util/Assert.h"
#include <GL/glew.h>
#include <string>
#include <boost/array.hpp>


class ShaderProgram;


namespace material {


/// XXX
class Variable
{
	public:
		/// The type
		enum Type
		{
			USER,
			BUILDIN
		};

		/// XXX Used for initialization in the constructor
		typedef boost::array<const ShaderProgram*, PASS_TYPES_NUM>
			ShaderPrograms;

		/// XXX
		typedef boost::array<const shader_program::Variable*,
			PASS_TYPES_NUM> ShaderProgramVariables;

		/// XXX
		Variable(
			Type type,
			const char* shaderProgVarName,
			const ShaderPrograms& shaderProgsArr);

		/// @name Accessors
		/// @{
		GETTER_R_BY_VAL(Type, type, getType)

		/// XXX
		const shader_program::Variable& getShaderProgramVariable(
			PassType p) const;

		/// XXX
		bool inPass(PassType p) const {return sProgsVars[p] != NULL;}

		/// Get the GL data type of all the shader program variables
		GLenum getGlDataType() const {return oneSProgVar->getGlDataType();}

		/// Get the name of all the shader program variables
		const char* getName() const {return oneSProgVar->getName().c_str();}

		/// Get the type of all the shader program variables
		shader_program::Variable::Type getShaderProgramVariableType() const
			{return oneSProgVar->getType();}
		/// @}

	private:
		Type type;
		ShaderProgramVariables sProgsVars;

		/// Keep one ShaderProgramVariable here for easy access of the common
		/// variable stuff like name or GL data type etc
		const shader_program::Variable* oneSProgVar;
};


inline const shader_program::Variable& Variable::getShaderProgramVariable(
	PassType p) const
{
	ASSERT(inPass(p));
	return *sProgsVars[p];
}


} // end namespace


#endif
