#include "UserMaterialVariable.h"
#include "UniformShaderProgramVariable.h"
#include "Texture.h"


//==============================================================================
// Constructors & destructor                                                   =
//==============================================================================

UserMaterialVariable::UserMaterialVariable(
	const char* shaderProgVarName,
	const ShaderPrograms& shaderProgsArr,
	float val)
:	MaterialVariable(USER, shaderProgVarName, shaderProgsArr)
{
	ASSERT(getGlDataType() == GL_FLOAT);
	ASSERT(getShaderProgramVariableType() == ShaderProgramVariable::UNIFORM);
	data = val;
}


UserMaterialVariable::UserMaterialVariable(
	const char* shaderProgVarName,
	const ShaderPrograms& shaderProgsArr,
	const Vec2& val)
:	MaterialVariable(USER, shaderProgVarName, shaderProgsArr)
{
	ASSERT(getGlDataType() == GL_FLOAT_VEC2);
	ASSERT(getShaderProgramVariableType() == ShaderProgramVariable::UNIFORM);
	data = val;
}


UserMaterialVariable::UserMaterialVariable(
	const char* shaderProgVarName,
	const ShaderPrograms& shaderProgsArr,
	const Vec3& val)
:	MaterialVariable(USER, shaderProgVarName, shaderProgsArr)
{
	ASSERT(getGlDataType() == GL_FLOAT_VEC3);
	ASSERT(getShaderProgramVariableType() == ShaderProgramVariable::UNIFORM);
	data = val;
}


UserMaterialVariable::UserMaterialVariable(
	const char* shaderProgVarName,
	const ShaderPrograms& shaderProgsArr,
	const Vec4& val)
:	MaterialVariable(USER, shaderProgVarName, shaderProgsArr)
{
	ASSERT(getGlDataType() == GL_FLOAT_VEC4);
	ASSERT(getShaderProgramVariableType() == ShaderProgramVariable::UNIFORM);
	data = val;
}


UserMaterialVariable::UserMaterialVariable(
	const char* shaderProgVarName,
	const ShaderPrograms& shaderProgsArr,
	const char* texFilename)
:	MaterialVariable(USER, shaderProgVarName, shaderProgsArr)
{
	ASSERT(getGlDataType() == GL_SAMPLER_2D);
	ASSERT(getShaderProgramVariableType() == ShaderProgramVariable::UNIFORM);
	data = RsrcPtr<Texture>();
	boost::get<RsrcPtr<Texture> >(data).loadRsrc(texFilename);
}


UserMaterialVariable::~UserMaterialVariable()
{}
