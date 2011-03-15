#include "MtlUserDefinedVar.h"
#include "Texture.h"


//======================================================================================================================
// Constructors                                                                                                        =
//======================================================================================================================

MtlUserDefinedVar::MtlUserDefinedVar(const SProgUniVar& sProgVar, Fai fai_):
	sProgVar(sProgVar)
{
	ASSERT(sProgVar.getGlDataType() == GL_SAMPLER_2D);
	data = fai_;
}


MtlUserDefinedVar::MtlUserDefinedVar(const SProgUniVar& sProgVar, float f):
	sProgVar(sProgVar)
{
	ASSERT(sProgVar.getGlDataType() == GL_FLOAT);
	data = f;
}


MtlUserDefinedVar::MtlUserDefinedVar(const SProgUniVar& sProgVar, const Vec2& v):
	sProgVar(sProgVar)
{
	ASSERT(sProgVar.getGlDataType() == GL_FLOAT_VEC2);
	data = v;
}


MtlUserDefinedVar::MtlUserDefinedVar(const SProgUniVar& sProgVar, const Vec3& v):
	sProgVar(sProgVar)
{
	ASSERT(sProgVar.getGlDataType() == GL_FLOAT_VEC3);
	data = v;
}


MtlUserDefinedVar::MtlUserDefinedVar(const SProgUniVar& sProgVar, const Vec4& v):
	sProgVar(sProgVar)
{
	ASSERT(sProgVar.getGlDataType() == GL_FLOAT_VEC4);
	data = v;
}


MtlUserDefinedVar::MtlUserDefinedVar(const SProgUniVar& sProgVar, const char* texFilename):
	sProgVar(sProgVar)
{
	ASSERT(sProgVar.getGlDataType() == GL_SAMPLER_2D);
	data = RsrcPtr<Texture>();
	boost::get<RsrcPtr<Texture> >(data).loadRsrc(texFilename);
}
