#include "ShaderProgramUniformVariable.h"
#include "resource/ShaderProgram.h"
#include "resource/Texture.h"
#include "gl/GlStateMachine.h"


//==============================================================================
// doSanityChecks                                                              =
//==============================================================================
void ShaderProgramUniformVariable::doSanityChecks() const
{
	ASSERT(getLocation() != -1);
	ASSERT(GlStateMachineSingleton::get().getCurrentProgramGlId() ==
		getFatherSProg().getGlId());
	ASSERT(glGetUniformLocation(getFatherSProg().getGlId(),
		getName().c_str()) == getLocation());
}


//==============================================================================
// set uniforms                                                                =
//==============================================================================

void ShaderProgramUniformVariable::set(const float f) const
{
	doSanityChecks();
	ASSERT(getGlDataType() == GL_FLOAT);

	glUniform1f(getLocation(), f);
}

void ShaderProgramUniformVariable::set(const float f[], uint size) const
{
	doSanityChecks();
	ASSERT(getGlDataType() == GL_FLOAT);

	if(size == 1)
	{
		glUniform1f(getLocation(), f[0]);
	}
	else
	{
		glUniform1fv(getLocation(), size, f);
	}
}


void ShaderProgramUniformVariable::set(const Vec2 v2[], uint size) const
{
	doSanityChecks();
	ASSERT(getGlDataType() == GL_FLOAT_VEC2);
	if(size == 1)
	{
		glUniform2f(getLocation(), v2[0].x(), v2[0].y());
	}
	else
	{
		glUniform2fv(getLocation(), size, &(const_cast<Vec2&>(v2[0]))[0]);
	}
}


void ShaderProgramUniformVariable::set(const Vec3 v3[], uint size) const
{
	doSanityChecks();
	ASSERT(getGlDataType() == GL_FLOAT_VEC3);

	if(size == 1)
	{
		glUniform3f(getLocation(), v3[0].x(), v3[0].y(), v3[0].z());
	}
	else
	{
		glUniform3fv(getLocation(), size, &(const_cast<Vec3&>(v3[0]))[0]);
	}
}


void ShaderProgramUniformVariable::set(const Vec4 v4[], uint size) const
{
	doSanityChecks();
	ASSERT(getGlDataType() == GL_FLOAT_VEC4);
	glUniform4fv(getLocation(), size, &(const_cast<Vec4&>(v4[0]))[0]);
}


void ShaderProgramUniformVariable::set(const Mat3 m3[], uint size) const
{
	doSanityChecks();
	ASSERT(getGlDataType() == GL_FLOAT_MAT3);
	glUniformMatrix3fv(getLocation(), size, true, &(m3[0])[0]);
}


void ShaderProgramUniformVariable::set(const Mat4 m4[], uint size) const
{
	doSanityChecks();
	ASSERT(getGlDataType() == GL_FLOAT_MAT4);
	glUniformMatrix4fv(getLocation(), size, true, &(m4[0])[0]);
}


void ShaderProgramUniformVariable::set(const Texture& tex, uint texUnit) const
{
	doSanityChecks();
	ASSERT(getGlDataType() == GL_SAMPLER_2D ||
		getGlDataType() == GL_SAMPLER_2D_SHADOW);
	tex.bind(texUnit);
	glUniform1i(getLocation(), texUnit);
}
