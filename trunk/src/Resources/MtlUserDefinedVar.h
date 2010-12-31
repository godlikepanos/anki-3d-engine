#ifndef MTL_USER_DEFINED_VAR_H
#define MTL_USER_DEFINED_VAR_H

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

	PROPERTY_R(float, float_, getFloat)
	PROPERTY_R(Vec2, vec2, getVec2)
	PROPERTY_R(Vec3, vec3, getVec3)
	PROPERTY_R(Vec4, vec4, getVec4)
	PROPERTY_R(Fai, fai, getFai)

	public:
		MtlUserDefinedVar(const SProgUniVar& sProgVar, const char* texFilename);
		MtlUserDefinedVar(const SProgUniVar& sProgVar, Fai fai);
		MtlUserDefinedVar(const SProgUniVar& sProgVar, float f);
		MtlUserDefinedVar(const SProgUniVar& sProgVar, const Vec2& v);
		MtlUserDefinedVar(const SProgUniVar& sProgVar, const Vec3& v);
		MtlUserDefinedVar(const SProgUniVar& sProgVar, const Vec4& v);

		const SProgUniVar& getUniVar() const {return sProgVar;}
		const Texture* getTexture() const {return texture.get();}

	private:
		const SProgUniVar& sProgVar;
		RsrcPtr<Texture> texture;
};


inline MtlUserDefinedVar::MtlUserDefinedVar(const SProgUniVar& sProgVar, Fai fai_):
	fai(fai_),
	sProgVar(sProgVar)
{}


inline MtlUserDefinedVar::MtlUserDefinedVar(const SProgUniVar& sProgVar, float f):
	float_(f),
	sProgVar(sProgVar)
{}


inline MtlUserDefinedVar::MtlUserDefinedVar(const SProgUniVar& sProgVar, const Vec2& v):
	vec2(v),
	sProgVar(sProgVar)
{}


inline MtlUserDefinedVar::MtlUserDefinedVar(const SProgUniVar& sProgVar, const Vec3& v):
	vec3(v),
	sProgVar(sProgVar)
{}


inline MtlUserDefinedVar::MtlUserDefinedVar(const SProgUniVar& sProgVar, const Vec4& v):
	vec4(v),
	sProgVar(sProgVar)
{}


#endif
