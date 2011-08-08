#include "UserVariable.h"
#include "../Texture.h"


namespace material {


//==============================================================================
// Constructors & destructor                                                   =
//==============================================================================

UserVariable::UserVariable(
	const char* shaderProgVarName,
	const ShaderPrograms& shaderProgsArr,
	float val)
:	Variable(USER, shaderProgVarName, shaderProgsArr)
{
	ASSERT(getGlDataType() == GL_FLOAT);
	ASSERT(getShaderProgramVariableType() ==
		shader_program::Variable::UNIFORM);
	data = val;
}


UserVariable::UserVariable(
	const char* shaderProgVarName,
	const ShaderPrograms& shaderProgsArr,
	const Vec2& val)
:	Variable(USER, shaderProgVarName, shaderProgsArr)
{
	ASSERT(getGlDataType() == GL_FLOAT_VEC2);
	ASSERT(getShaderProgramVariableType() ==
		shader_program::Variable::UNIFORM);
	data = val;
}


UserVariable::UserVariable(
	const char* shaderProgVarName,
	const ShaderPrograms& shaderProgsArr,
	const Vec3& val)
:	Variable(USER, shaderProgVarName, shaderProgsArr)
{
	ASSERT(getGlDataType() == GL_FLOAT_VEC3);
	ASSERT(getShaderProgramVariableType() ==
		shader_program::Variable::UNIFORM);
	data = val;
}


UserVariable::UserVariable(
	const char* shaderProgVarName,
	const ShaderPrograms& shaderProgsArr,
	const Vec4& val)
:	Variable(USER, shaderProgVarName, shaderProgsArr)
{
	ASSERT(getGlDataType() == GL_FLOAT_VEC4);
	ASSERT(getShaderProgramVariableType() ==
		shader_program::Variable::UNIFORM);
	data = val;
}


UserVariable::UserVariable(
	const char* shaderProgVarName,
	const ShaderPrograms& shaderProgsArr,
	const char* texFilename)
:	Variable(USER, shaderProgVarName, shaderProgsArr)
{
	ASSERT(getGlDataType() == GL_SAMPLER_2D);
	ASSERT(getShaderProgramVariableType() ==
		shader_program::Variable::UNIFORM);
	data = RsrcPtr<Texture>();
	boost::get<RsrcPtr<Texture> >(data).loadRsrc(texFilename);
}


UserVariable::~UserVariable()
{}


} // end namespace
