#ifndef SHADER_PROGRAM_ATTRIBUTE_VARIABLE_H
#define SHADER_PROGRAM_ATTRIBUTE_VARIABLE_H

#include "Variable.h"


namespace shader_program {


/// Attribute shader program variable
class AttributeVariable: public Variable
{
	public:
		AttributeVariable(
			int loc, const char* name, GLenum glDataType,
			const ShaderProgram& fatherSProg);
};


inline AttributeVariable::AttributeVariable(
	int loc, const char* name,
	GLenum glDataType, const ShaderProgram& fatherSProg)
:	Variable(loc, name, glDataType, ATTRIBUTE, fatherSProg)
{}


} // end namespace


#endif
