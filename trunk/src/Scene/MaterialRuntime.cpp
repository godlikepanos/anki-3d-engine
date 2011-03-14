#include "MaterialRuntime.h"


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
MtlUserDefinedVarRuntime::MtlUserDefinedVarRuntime(const MtlUserDefinedVar& rsrc_):
	rsrc(rsrc_)
{
	switch(rsrc.getUniVar().getGlDataType())
	{
		case GL_FLOAT:
			break;
		case GL_FLOAT_VEC2:
			break;
		case GL_FLOAT_VEC3:
			break;
		case GL_FLOAT_VEC4:
			break;
		case GL_SAMPLER_2D:
			break;
		default:
			ASSERT(0);
	}
}
