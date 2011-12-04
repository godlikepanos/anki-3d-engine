#include "anki/resource/ShaderProgramUniformVariable.h"
#include "anki/resource/ShaderProgram.h"
#include "anki/resource/Texture.h"
#include "anki/gl/GlStateMachine.h"


namespace anki {


//==============================================================================
void ShaderProgramUniformVariable::doSanityChecks() const
{
	ANKI_ASSERT(getLocation() != -1);
	ANKI_ASSERT(GlStateMachineSingleton::get().getCurrentProgramGlId() ==
		getFatherSProg().getGlId());
	ANKI_ASSERT(glGetUniformLocation(getFatherSProg().getGlId(),
		getName().c_str()) == getLocation());
}


//==============================================================================
// set uniforms                                                                =
//==============================================================================

void ShaderProgramUniformVariable::set(const float x) const
{
	doSanityChecks();
	ANKI_ASSERT(getGlDataType() == GL_FLOAT);

	glUniform1f(getLocation(), x);
}


void ShaderProgramUniformVariable::set(const Vec2& x) const
{
	glUniform2f(getLocation(), x.x(), x.y());
}


void ShaderProgramUniformVariable::set(const float x[], uint size) const
{
	doSanityChecks();
	ANKI_ASSERT(getGlDataType() == GL_FLOAT);
	ANKI_ASSERT(size > 1);
	glUniform1fv(getLocation(), size, x);
}


void ShaderProgramUniformVariable::set(const Vec2 x[], uint size) const
{
	doSanityChecks();
	ANKI_ASSERT(getGlDataType() == GL_FLOAT_VEC2);
	ANKI_ASSERT(size > 1);
	glUniform2fv(getLocation(), size, &(const_cast<Vec2&>(x[0]))[0]);
}


void ShaderProgramUniformVariable::set(const Vec3 x[], uint size) const
{
	doSanityChecks();
	ANKI_ASSERT(getGlDataType() == GL_FLOAT_VEC3);
	ANKI_ASSERT(size > 0);
	glUniform3fv(getLocation(), size, &(const_cast<Vec3&>(x[0]))[0]);
}


void ShaderProgramUniformVariable::set(const Vec4 x[], uint size) const
{
	doSanityChecks();
	ANKI_ASSERT(getGlDataType() == GL_FLOAT_VEC4);
	ANKI_ASSERT(size > 0);
	glUniform4fv(getLocation(), size, &(const_cast<Vec4&>(x[0]))[0]);
}


void ShaderProgramUniformVariable::set(const Mat3 x[], uint size) const
{
	doSanityChecks();
	ANKI_ASSERT(getGlDataType() == GL_FLOAT_MAT3);
	ANKI_ASSERT(size > 0);
	glUniformMatrix3fv(getLocation(), size, true, &(x[0])[0]);
}


void ShaderProgramUniformVariable::set(const Mat4 x[], uint size) const
{
	doSanityChecks();
	ANKI_ASSERT(getGlDataType() == GL_FLOAT_MAT4);
	ANKI_ASSERT(size > 0);
	glUniformMatrix4fv(getLocation(), size, true, &(x[0])[0]);
}


void ShaderProgramUniformVariable::set(const Texture& tex, uint texUnit) const
{
	doSanityChecks();
	ANKI_ASSERT(getGlDataType() == GL_SAMPLER_2D ||
		getGlDataType() == GL_SAMPLER_2D_SHADOW);
	tex.bind(texUnit);
	glUniform1i(getLocation(), texUnit);
}


} // end namespace
