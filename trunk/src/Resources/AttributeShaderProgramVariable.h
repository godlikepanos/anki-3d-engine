#ifndef ATTRIBUTE_SHADER_PROGRAM_VARIABLE_H
#define ATTRIBUTE_SHADER_PROGRAM_VARIABLE_H

#include "ShaderProgramVariable.h"


/// Attribute shader program variable
class AttributeShaderProgramVariable: public ShaderProgramVariable
{
	public:
		AttributeShaderProgramVariable(
			int loc_, const char* name_, GLenum glDataType_,
			const ShaderProgram& fatherSProg_);
};


inline AttributeShaderProgramVariable::AttributeShaderProgramVariable(
	int loc_, const char* name_,
	GLenum glDataType_, const ShaderProgram& fatherSProg_)
:	ShaderProgramVariable(loc_, name_, glDataType_, ATTRIBUTE, fatherSProg_)
{}


#endif
