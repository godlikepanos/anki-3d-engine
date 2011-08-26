#ifndef MATERIAL_VARIABLE_H
#define MATERIAL_VARIABLE_H

#include "MaterialCommon.h"
#include "ShaderProgramVariable.h"
#include "util/Accessors.h"
#include "util/Assert.h"
#include <GL/glew.h>
#include <string>
#include <boost/array.hpp>


class ShaderProgram;


/// XXX
class MaterialVariable
{
	public:
		/// The type
		enum Type
		{
			T_USER,
			T_BUILDIN
		};

		/// XXX Used for initialization in the constructor
		typedef boost::array<const ShaderProgram*, PASS_TYPES_NUM>
			ShaderPrograms;

		/// XXX
		typedef boost::array<const ShaderProgramVariable*,
			PASS_TYPES_NUM> ShaderProgramVariables;

		/// XXX
		MaterialVariable(
			Type type,
			const char* shaderProgVarName,
			const ShaderPrograms& shaderProgsArr);

		/// @name Accessors
		/// @{
		GETTER_R_BY_VAL(Type, type, getType)

		/// XXX
		const ShaderProgramVariable& getShaderProgramVariable(
			PassType p) const;

		/// XXX
		bool inPass(PassType p) const {return sProgVars[p] != NULL;}

		/// Get the GL data type of all the shader program variables
		GLenum getGlDataType() const {return oneSProgVar->getGlDataType();}

		/// Get the name of all the shader program variables
		const char* getName() const {return oneSProgVar->getName().c_str();}

		/// Get the type of all the shader program variables
		ShaderProgramVariable::Type getShaderProgramVariableType() const
			{return oneSProgVar->getType();}
		/// @}

	private:
		Type type;
		ShaderProgramVariables sProgVars;

		/// Keep one ShaderProgramVariable here for easy access of the common
		/// variable stuff like name or GL data type etc
		const ShaderProgramVariable* oneSProgVar;
};


inline const ShaderProgramVariable& MaterialVariable::getShaderProgramVariable(
	PassType p) const
{
	ASSERT(inPass(p));
	return *sProgVars[p];
}


#endif
