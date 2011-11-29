#ifndef ANKI_RESOURCE_SHADER_PROGRAM_ATTRIBUTE_VARIABLE_H
#define ANKI_RESOURCE_SHADER_PROGRAM_ATTRIBUTE_VARIABLE_H

#include "anki/resource/ShaderProgramVariable.h"


namespace anki {


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
	: ShaderProgramVariable(loc, name, glDataType, T_ATTRIBUTE, fatherSProg)
{}


} // end namespace


#endif
