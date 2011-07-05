#include "MtlUserDefinedVar.h"
#include "Resources/Texture.h"


//==============================================================================
// Constructors                                                                =
//==============================================================================

template<>
MtlUserDefinedVar::MtlUserDefinedVar(const SProgUniVar& var, const Fai& fai_)
:	sProgVar(var)
{
	ASSERT(var.getGlDataType() == GL_SAMPLER_2D);
	data = fai_;
}


template<>
MtlUserDefinedVar::MtlUserDefinedVar(const SProgUniVar& var, const float& f)
:	sProgVar(var)
{
	ASSERT(var.getGlDataType() == GL_FLOAT);
	data = f;
}


template<>
MtlUserDefinedVar::MtlUserDefinedVar(const SProgUniVar& var, const Vec2& v)
:	sProgVar(var)
{
	ASSERT(var.getGlDataType() == GL_FLOAT_VEC2);
	data = v;
}


template<>
MtlUserDefinedVar::MtlUserDefinedVar(const SProgUniVar& var, const Vec3& v)
:	sProgVar(var)
{
	ASSERT(var.getGlDataType() == GL_FLOAT_VEC3);
	data = v;
}


template<>
MtlUserDefinedVar::MtlUserDefinedVar(const SProgUniVar& var, const Vec4& v)
:	sProgVar(var)
{
	ASSERT(var.getGlDataType() == GL_FLOAT_VEC4);
	data = v;
}


template<>
MtlUserDefinedVar::MtlUserDefinedVar(const SProgUniVar& var,
	const std::string& texFilename)
:	sProgVar(var)
{
	ASSERT(var.getGlDataType() == GL_SAMPLER_2D);
	data = RsrcPtr<Texture>();
	boost::get<RsrcPtr<Texture> >(data).loadRsrc(texFilename.c_str());
}
