#include "anki/resource/MaterialUserVariable.h"
#include "anki/resource/Texture.h"


//==============================================================================
// Constructors & destructor                                                   =
//==============================================================================

MaterialUserVariable::MaterialUserVariable(
	const char* shaderProgVarName,
	const ShaderPrograms& shaderProgsArr,
	float val)
:	MaterialVariable(T_USER, shaderProgVarName, shaderProgsArr)
{
	ASSERT(getGlDataType() == GL_FLOAT);
	ASSERT(getShaderProgramVariableType() ==
		ShaderProgramVariable::T_UNIFORM);
	data = val;
}


MaterialUserVariable::MaterialUserVariable(
	const char* shaderProgVarName,
	const ShaderPrograms& shaderProgsArr,
	const Vec2& val)
:	MaterialVariable(T_USER, shaderProgVarName, shaderProgsArr)
{
	ASSERT(getGlDataType() == GL_FLOAT_VEC2);
	ASSERT(getShaderProgramVariableType() ==
		ShaderProgramVariable::T_UNIFORM);
	data = val;
}


MaterialUserVariable::MaterialUserVariable(
	const char* shaderProgVarName,
	const ShaderPrograms& shaderProgsArr,
	const Vec3& val)
:	MaterialVariable(T_USER, shaderProgVarName, shaderProgsArr)
{
	ASSERT(getGlDataType() == GL_FLOAT_VEC3);
	ASSERT(getShaderProgramVariableType() ==
		ShaderProgramVariable::T_UNIFORM);
	data = val;
}


MaterialUserVariable::MaterialUserVariable(
	const char* shaderProgVarName,
	const ShaderPrograms& shaderProgsArr,
	const Vec4& val)
:	MaterialVariable(T_USER, shaderProgVarName, shaderProgsArr)
{
	ASSERT(getGlDataType() == GL_FLOAT_VEC4);
	ASSERT(getShaderProgramVariableType() ==
		ShaderProgramVariable::T_UNIFORM);
	data = val;
}


MaterialUserVariable::MaterialUserVariable(
	const char* shaderProgVarName,
	const ShaderPrograms& shaderProgsArr,
	const char* texFilename)
:	MaterialVariable(T_USER, shaderProgVarName, shaderProgsArr)
{
	ASSERT(getGlDataType() == GL_SAMPLER_2D);
	ASSERT(getShaderProgramVariableType() ==
		ShaderProgramVariable::T_UNIFORM);
	data = RsrcPtr<Texture>();
	boost::get<RsrcPtr<Texture> >(data).loadRsrc(texFilename);
}


MaterialUserVariable::~MaterialUserVariable()
{}
