#include "MaterialInputVariable.h"
#include "SProgUniVar.h"
#include "Texture.h"
#include "Util/Assert.h"


//==============================================================================
// Constructor                                                                 =
//==============================================================================

MaterialInputVariable::MaterialInputVariable(const SProgUniVar& svar, float val)
:	sProgVar(svar)
{
	ASSERT(svar.getGlDataType() == GL_FLOAT);
	scalar = val;
	type = T_FLOAT;
}


MaterialInputVariable::MaterialInputVariable(const SProgUniVar& svar,
	const Vec2& val)
:	sProgVar(svar)
{
	ASSERT(svar.getGlDataType() == GL_FLOAT_VEC2);
	vec2 = val;
	type = T_VEC2;
}


MaterialInputVariable::MaterialInputVariable(const SProgUniVar& svar,
	const Vec3& val)
:	sProgVar(svar)
{
	ASSERT(svar.getGlDataType() == GL_FLOAT_VEC3);
	vec3 = val;
	type = T_VEC3;
}


MaterialInputVariable::MaterialInputVariable(const SProgUniVar& svar,
	const Vec4& val)
:	sProgVar(svar)
{
	ASSERT(svar.getGlDataType() == GL_FLOAT_VEC4);
	vec4 = val;
	type = T_VEC4;
}


MaterialInputVariable::MaterialInputVariable(const SProgUniVar& svar, Fai val)
:	sProgVar(svar)
{
	ASSERT(svar.getGlDataType() == GL_SAMPLER_2D);
	fai = val;
	type = T_FAI;
}


MaterialInputVariable::MaterialInputVariable(const SProgUniVar& svar,
	const char* texFilename)
:	sProgVar(svar)
{
	ASSERT(svar.getGlDataType() == GL_SAMPLER_2D);
	texture.loadRsrc(texFilename);
	type = T_TEXTURE;
}


//==============================================================================
// Destructor                                                                  =
//==============================================================================
MaterialInputVariable::~MaterialInputVariable()
{}


//==============================================================================
// Accessors                                                                   =
//==============================================================================

float MaterialInputVariable::getFloat() const
{
	ASSERT(sProgVar.getGlDataType() == GL_FLOAT);
	return scalar;
}


const Vec2& MaterialInputVariable::getVec2() const
{
	ASSERT(sProgVar.getGlDataType() == GL_FLOAT_VEC2);
	return vec2;
}


const Vec3& MaterialInputVariable::getVec3() const
{
	ASSERT(sProgVar.getGlDataType() == GL_FLOAT_VEC3);
	return vec3;
}


const Vec4& MaterialInputVariable::getVec4() const
{
	ASSERT(sProgVar.getGlDataType() == GL_FLOAT_VEC4);
	return vec4;
}


MaterialInputVariable::Fai MaterialInputVariable::getFai() const
{
	ASSERT(sProgVar.getGlDataType() == GL_SAMPLER_2D);
	return fai;
}


const Texture& MaterialInputVariable::getTexture() const
{
	ASSERT(sProgVar.getGlDataType() == GL_SAMPLER_2D);
	return *texture;
}

















