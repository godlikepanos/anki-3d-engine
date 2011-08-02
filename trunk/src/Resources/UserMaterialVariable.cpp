#include "UserMaterialVariable.h"
#include "UniformShaderProgramVariable.h"
#include "Texture.h"


//==============================================================================
// Constructors & destructor                                                   =
//==============================================================================

UserMaterialVariable::UserMaterialVariable(
	const UniformShaderProgramVariable* cpUni,
	const UniformShaderProgramVariable* dpUni,
	float val)
:	MaterialVariable(T_USER, cpUni, dpUni)
{
	ASSERT(getGlDataType() == GL_FLOAT);
	data = val;
}


UserMaterialVariable::UserMaterialVariable(
	const UniformShaderProgramVariable* cpUni,
	const UniformShaderProgramVariable* dpUni,
	const Vec2& val)
:	MaterialVariable(T_USER, cpUni, dpUni)
{
	ASSERT(getGlDataType() == GL_FLOAT_VEC2);
	data = val;
}


UserMaterialVariable::UserMaterialVariable(
	const UniformShaderProgramVariable* cpUni,
	const UniformShaderProgramVariable* dpUni,
	const Vec3& val)
:	MaterialVariable(T_USER, cpUni, dpUni)
{
	ASSERT(getGlDataType() == GL_FLOAT_VEC3);
	data = val;
}


UserMaterialVariable::UserMaterialVariable(
	const UniformShaderProgramVariable* cpUni,
	const UniformShaderProgramVariable* dpUni,
	const Vec4& val)
:	MaterialVariable(T_USER, cpUni, dpUni)
{
	ASSERT(getGlDataType() == GL_FLOAT_VEC4);
	data = val;
}


UserMaterialVariable::UserMaterialVariable(
	const UniformShaderProgramVariable* cpUni,
	const UniformShaderProgramVariable* dpUni,
	const char* texFilename)
:	MaterialVariable(T_USER, cpUni, dpUni)
{
	ASSERT(getGlDataType() == GL_SAMPLER_2D);
	data = RsrcPtr<Texture>();
	boost::get<RsrcPtr<Texture> >(data).loadRsrc(texFilename);
}


UserMaterialVariable::~UserMaterialVariable()
{}


//==============================================================================
// Accessors                                                                   =
//==============================================================================

const UniformShaderProgramVariable&
	UserMaterialVariable::getColorPassUniformShaderProgramVariable() const
{
	return static_cast<const UniformShaderProgramVariable&>(
		getColorPassShaderProgramVariable());
}


const UniformShaderProgramVariable&
	UserMaterialVariable::getDepthPassUniformShaderProgramVariable() const
{
	return static_cast<const UniformShaderProgramVariable&>(
		getDepthPassShaderProgramVariable());
}
