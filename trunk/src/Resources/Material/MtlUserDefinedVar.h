#ifndef MTL_USER_DEFINED_VAR_H
#define MTL_USER_DEFINED_VAR_H

#include <boost/variant.hpp>
#include "Properties.h"
#include "Math.h"
#include "SProgUniVar.h"
#include "RsrcPtr.h"


class Texture;


/// Class for user defined material variables that will be passes in to the shader
class MtlUserDefinedVar
{
	public:
		/// The renderer's FAIs
		enum Fai
		{
			MS_DEPTH_FAI, ///< Avoid it in MS
			IS_FAI, ///< Use it anywhere
			PPS_PRE_PASS_FAI, ///< Avoid it in BS
			PPS_POST_PASS_FAI ///< Use it anywhere
		};

		/// The data union
		typedef boost::variant<float, Vec2, Vec3, Vec4, RsrcPtr<Texture>, Fai> DataVariant;

		MtlUserDefinedVar(const SProgUniVar& sProgVar, const char* texFilename);
		MtlUserDefinedVar(const SProgUniVar& sProgVar, Fai fai);
		MtlUserDefinedVar(const SProgUniVar& sProgVar, float f);
		MtlUserDefinedVar(const SProgUniVar& sProgVar, const Vec2& v);
		MtlUserDefinedVar(const SProgUniVar& sProgVar, const Vec3& v);
		MtlUserDefinedVar(const SProgUniVar& sProgVar, const Vec4& v);

		/// @name Accessors
		/// @{
		const SProgUniVar& getUniVar() const {return sProgVar;}

		const DataVariant& getDataVariant() const {return data;}

		template<typename Type>
		const Type& get() const {return boost::get<Type>(data);}
		/// @}

	private:
		DataVariant data;
		const SProgUniVar& sProgVar; ///< Know the other resource
};


inline MtlUserDefinedVar::MtlUserDefinedVar(const SProgUniVar& sProgVar, Fai fai_):
	sProgVar(sProgVar)
{
	ASSERT(sProgVar.getGlDataType() == GL_SAMPLER_2D);
	data = fai_;
}


inline MtlUserDefinedVar::MtlUserDefinedVar(const SProgUniVar& sProgVar, float f):
	sProgVar(sProgVar)
{
	ASSERT(sProgVar.getGlDataType() == GL_FLOAT);
	data = f;
}


inline MtlUserDefinedVar::MtlUserDefinedVar(const SProgUniVar& sProgVar, const Vec2& v):
	sProgVar(sProgVar)
{
	ASSERT(sProgVar.getGlDataType() == GL_FLOAT_VEC2);
	data = v;
}


inline MtlUserDefinedVar::MtlUserDefinedVar(const SProgUniVar& sProgVar, const Vec3& v):
	sProgVar(sProgVar)
{
	ASSERT(sProgVar.getGlDataType() == GL_FLOAT_VEC3);
	data = v;
}


inline MtlUserDefinedVar::MtlUserDefinedVar(const SProgUniVar& sProgVar, const Vec4& v):
	sProgVar(sProgVar)
{
	ASSERT(sProgVar.getGlDataType() == GL_FLOAT_VEC4);
	data = v;
}


#endif
