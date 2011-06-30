#include "MtlUserDefinedVar.h"
#include "Resources/Texture.h"


//==============================================================================
// Constructors                                                                =
//==============================================================================

template<>
MtlUserDefinedVar::MtlUserDefinedVar(const SProgUniVar& sProgVar, const Fai& fai_):
	sProgVar(sProgVar)
{
	ASSERT(sProgVar.getGlDataType() == GL_SAMPLER_2D);
	data = fai_;
}


template<>
MtlUserDefinedVar::MtlUserDefinedVar(const SProgUniVar& sProgVar, const float& f):
	sProgVar(sProgVar)
{
	ASSERT(sProgVar.getGlDataType() == GL_FLOAT);
	data = f;
}


template<>
MtlUserDefinedVar::MtlUserDefinedVar(const SProgUniVar& sProgVar, const Vec2& v):
	sProgVar(sProgVar)
{
	ASSERT(sProgVar.getGlDataType() == GL_FLOAT_VEC2);
	data = v;
}


template<>
MtlUserDefinedVar::MtlUserDefinedVar(const SProgUniVar& sProgVar, const Vec3& v):
	sProgVar(sProgVar)
{
	ASSERT(sProgVar.getGlDataType() == GL_FLOAT_VEC3);
	data = v;
}


template<>
MtlUserDefinedVar::MtlUserDefinedVar(const SProgUniVar& sProgVar, const Vec4& v):
	sProgVar(sProgVar)
{
	ASSERT(sProgVar.getGlDataType() == GL_FLOAT_VEC4);
	data = v;
}


template<>
MtlUserDefinedVar::MtlUserDefinedVar(const SProgUniVar& sProgVar, const std::string& texFilename):
	sProgVar(sProgVar)
{
	ASSERT(sProgVar.getGlDataType() == GL_SAMPLER_2D);
	data = RsrcPtr<Texture>();
	boost::get<RsrcPtr<Texture> >(data).loadRsrc(texFilename.c_str());
}
