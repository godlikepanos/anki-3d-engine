#include "UserMaterialVariableRuntime.h"
#include "Resources/UserMaterialVariable.h"


//==============================================================================
// Constructor                                                                =
//==============================================================================
UserMaterialVariableRuntime::UserMaterialVariableRuntime(
	const UserMaterialVariable& umv_)
:	umv(umv_)
{
	switch(umv.getGlDataType())
	{
		case GL_FLOAT:
			data.scalar = umv.getFloat();
			break;
		case GL_FLOAT_VEC2:
			data.vec2 = umv.getVec2();
			break;
		case GL_FLOAT_VEC3:
			data.vec3 = umv.getVec3();
			break;
		case GL_FLOAT_VEC4:
			data.vec4 = umv.getVec4();
			break;
		case GL_SAMPLER_2D:
			data.tex = &umv.getTexture();
			break;
		default:
			ASSERT(1);
	}
}
