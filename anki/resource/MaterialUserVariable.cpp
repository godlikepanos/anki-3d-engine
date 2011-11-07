#include "anki/resource/MaterialUserVariable.h"
#include "anki/resource/Texture.h"


namespace anki {


//==============================================================================
// Constructors & destructor                                                   =
//==============================================================================

MaterialUserVariable::MaterialUserVariable(
	const char* shaderProgVarName,
	const ShaderPrograms& shaderProgsArr,
	float val)
:	MaterialVariable(T_USER, shaderProgVarName, shaderProgsArr)
{
	ANKI_ASSERT(getGlDataType() == GL_FLOAT);
	ANKI_ASSERT(getShaderProgramVariableType() ==
		ShaderProgramVariable::T_UNIFORM);
	data = val;
}


MaterialUserVariable::MaterialUserVariable(
	const char* shaderProgVarName,
	const ShaderPrograms& shaderProgsArr,
	const Vec2& val)
:	MaterialVariable(T_USER, shaderProgVarName, shaderProgsArr)
{
	ANKI_ASSERT(getGlDataType() == GL_FLOAT_VEC2);
	ANKI_ASSERT(getShaderProgramVariableType() ==
		ShaderProgramVariable::T_UNIFORM);
	data = val;
}


MaterialUserVariable::MaterialUserVariable(
	const char* shaderProgVarName,
	const ShaderPrograms& shaderProgsArr,
	const Vec3& val)
:	MaterialVariable(T_USER, shaderProgVarName, shaderProgsArr)
{
	ANKI_ASSERT(getGlDataType() == GL_FLOAT_VEC3);
	ANKI_ASSERT(getShaderProgramVariableType() ==
		ShaderProgramVariable::T_UNIFORM);
	data = val;
}


MaterialUserVariable::MaterialUserVariable(
	const char* shaderProgVarName,
	const ShaderPrograms& shaderProgsArr,
	const Vec4& val)
:	MaterialVariable(T_USER, shaderProgVarName, shaderProgsArr)
{
	ANKI_ASSERT(getGlDataType() == GL_FLOAT_VEC4);
	ANKI_ASSERT(getShaderProgramVariableType() ==
		ShaderProgramVariable::T_UNIFORM);
	data = val;
}


MaterialUserVariable::MaterialUserVariable(
	const char* shaderProgVarName,
	const ShaderPrograms& shaderProgsArr,
	const char* texFilename)
:	MaterialVariable(T_USER, shaderProgVarName, shaderProgsArr)
{
	ANKI_ASSERT(getGlDataType() == GL_SAMPLER_2D);
	ANKI_ASSERT(getShaderProgramVariableType() ==
		ShaderProgramVariable::T_UNIFORM);
	data = TextureResourcePointer();
	boost::get<TextureResourcePointer >(data).load(texFilename);
}


MaterialUserVariable::~MaterialUserVariable()
{}


} // end namespace
