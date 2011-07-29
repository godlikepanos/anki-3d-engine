#include "UserMaterialVariable.h"
#include "UniformShaderProgramVariable.h"
#include "Texture.h"


//==============================================================================
// Constructors                                                                =
//==============================================================================

UserMaterialVariable::UserMaterialVariable(
	const UniformShaderProgramVariable* cpUni,
	const UniformShaderProgramVariable* dpUni,
	float val)
:	MaterialVariable(T_USER, cpUni, dpUni)
{
	ASSERT(getGlDataType() == GL_FLOAT);
	data.scalar = val;
}


UserMaterialVariable::UserMaterialVariable(
	const UniformShaderProgramVariable* cpUni,
	const UniformShaderProgramVariable* dpUni,
	const Vec2& val)
:	MaterialVariable(T_USER, cpUni, dpUni)
{
	ASSERT(getGlDataType() == GL_FLOAT_VEC2);
	data.vec2 = val;
}


UserMaterialVariable::UserMaterialVariable(
	const UniformShaderProgramVariable* cpUni,
	const UniformShaderProgramVariable* dpUni,
	const Vec3& val)
:	MaterialVariable(T_USER, cpUni, dpUni)
{
	ASSERT(getGlDataType() == GL_FLOAT_VEC3);
	data.vec3 = val;
}


UserMaterialVariable::UserMaterialVariable(
	const UniformShaderProgramVariable* cpUni,
	const UniformShaderProgramVariable* dpUni,
	const Vec4& val)
:	MaterialVariable(T_USER, cpUni, dpUni)
{
	ASSERT(getGlDataType() == GL_FLOAT_VEC4);
	data.vec4 = val;
}


UserMaterialVariable::UserMaterialVariable(
	const UniformShaderProgramVariable* cpUni,
	const UniformShaderProgramVariable* dpUni,
	const char* texFilename)
:	MaterialVariable(T_USER, cpUni, dpUni)
{
	ASSERT(getGlDataType() == GL_SAMPLER_2D);
	data.texture.loadRsrc(texFilename);
}


//==============================================================================
// Accessors                                                                   =
//==============================================================================

float UserMaterialVariable::getFloat() const
{
	ASSERT(getGlDataType() == GL_FLOAT);
	return data.scalar;
}


const Vec2& UserMaterialVariable::getVec2() const
{
	ASSERT(getGlDataType() == GL_FLOAT_VEC2);
	return data.vec2;
}


const Vec3& UserMaterialVariable::getVec3() const
{
	ASSERT(getGlDataType() == GL_FLOAT_VEC3);
	return data.vec3;
}


const Vec4& UserMaterialVariable::getVec4() const
{
	ASSERT(getGlDataType() == GL_FLOAT_VEC4);
	return data.vec4;
}


const Texture& UserMaterialVariable::getTexture() const
{
	ASSERT(getGlDataType() == GL_SAMPLER_2D);
	return *data.texture;
}
