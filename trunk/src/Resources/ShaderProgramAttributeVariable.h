#ifndef SHADER_PROGRAM_ATTRIBUTE_VARIABLE_H
#define SHADER_PROGRAM_ATTRIBUTE_VARIABLE_H

#include "ShaderProgramVariable.h"


/// Attribute shader program variable
class ShaderProgramAttributeVariable: public ShaderProgramVariable
{
	public:
		ShaderProgramAttributeVariable(
			int loc, const char* name, GLenum glDataType,
			const ShaderProgram& fatherSProg);
};


inline ShaderProgramAttributeVariable::ShaderProgramAttributeVariable(
	int loc, const char* name,
	GLenum glDataType, const ShaderProgram& fatherSProg)
:	ShaderProgramVariable(loc, name, glDataType, ATTRIBUTE, fatherSProg)
{}


#endif
