#include "SProgUniVar.h"
#include "ShaderProg.h"
#include "Texture.h"


//======================================================================================================================
// set uniforms                                                                                                        =
//======================================================================================================================
/// Standard set uniform checks
/// - Check if initialized
/// - if the current shader program is the var's shader program
/// - if the GL driver gives the same location as the one the var has
#define STD_SET_UNI_CHECK() \
	ASSERT(getLoc() != -1); \
	ASSERT(ShaderProg::getCurrentProgramGlId() == getFatherSProg().getGlId()); \
	ASSERT(glGetUniformLocation(getFatherSProg().getGlId(), getName().c_str()) == getLoc());


void SProgUniVar::set(const float f[], uint size) const
{
	STD_SET_UNI_CHECK();
	ASSERT(getGlDataType() == GL_FLOAT);

	if(size == 1)
	{
		glUniform1f(getLoc(), f[0]);
	}
	else
	{
		glUniform1fv(getLoc(), size, f);
	}
}

void SProgUniVar::set(const Vec2 v2[], uint size) const
{
	STD_SET_UNI_CHECK();
	ASSERT(getGlDataType() == GL_FLOAT_VEC2);
	glUniform2fv(getLoc(), size, &(const_cast<Vec2&>(v2[0]))[0]);
}

void SProgUniVar::set(const Vec3 v3[], uint size) const
{
	STD_SET_UNI_CHECK();
	ASSERT(getGlDataType() == GL_FLOAT_VEC3);
	glUniform3fv(getLoc(), size, &(const_cast<Vec3&>(v3[0]))[0]);
}

void SProgUniVar::set(const Vec4 v4[], uint size) const
{
	STD_SET_UNI_CHECK();
	ASSERT(getGlDataType() == GL_FLOAT_VEC4);
	glUniform4fv(getLoc(), size, &(const_cast<Vec4&>(v4[0]))[0]);
}

void SProgUniVar::set(const Mat3 m3[], uint size) const
{
	STD_SET_UNI_CHECK();
	ASSERT(getGlDataType() == GL_FLOAT_MAT3);
	glUniformMatrix3fv(getLoc(), size, true, &(m3[0])[0]);
}

void SProgUniVar::set(const Mat4 m4[], uint size) const
{
	STD_SET_UNI_CHECK();
	ASSERT(getGlDataType() == GL_FLOAT_MAT4);
	glUniformMatrix4fv(getLoc(), size, true, &(m4[0])[0]);
}

void SProgUniVar::set(const Texture& tex, uint texUnit) const
{
	STD_SET_UNI_CHECK();
	ASSERT(getGlDataType() == GL_SAMPLER_2D || getGlDataType() == GL_SAMPLER_2D_SHADOW);
	tex.bind(texUnit);
	glUniform1i(getLoc(), texUnit);
}
