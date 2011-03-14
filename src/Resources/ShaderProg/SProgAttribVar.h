#ifndef S_PROG_ATTRIB_VAR_H
#define S_PROG_ATTRIB_VAR_H

#include "SProgVar.h"


/// Attribute shader variable
class SProgAttribVar: public SProgVar
{
	public:
		SProgAttribVar(int loc_, const char* name_, GLenum glDataType_, const ShaderProg* fatherSProg_);
		SProgAttribVar(const SProgAttribVar& var): SProgVar(var) {}
};


inline SProgAttribVar::SProgAttribVar(int loc_, const char* name_, GLenum glDataType_,
                                      const ShaderProg* fatherSProg_):
	SProgVar(loc_, name_, glDataType_, SVT_UNIFORM, fatherSProg_)
{}


#endif
