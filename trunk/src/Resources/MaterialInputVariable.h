#ifndef MATERIAL_INPUT_VARIABLE_H
#define MATERIAL_INPUT_VARIABLE_H

#include "Math/Math.h"
#include "RsrcPtr.h"
#include "Util/Accessors.h"


class Texture;
class SProgUniVar;


/// @todo
class MaterialInputVariable
{
	public:
		/// The type of the variable
		enum Type
		{
			T_FAI,
			T_TEXTURE,
			T_FLOAT,
			T_VEC2,
			T_VEC3,
			T_VEC4,
			T_NUM
		};

		/// The renderer's FAIs
		enum Fai
		{
			MS_DEPTH_FAI, ///< Avoid it in MS
			IS_FAI, ///< Use it anywhere
			PPS_PRE_PASS_FAI, ///< Avoid it in BS
			PPS_POST_PASS_FAI ///< Use it anywhere
		};

		/// @name Constructors
		/// @{
		MaterialInputVariable(const SProgUniVar& sProgVar, float val);
		MaterialInputVariable(const SProgUniVar& sProgVar, const Vec2& val);
		MaterialInputVariable(const SProgUniVar& sProgVar, const Vec3& val);
		MaterialInputVariable(const SProgUniVar& sProgVar, const Vec4& val);
		MaterialInputVariable(const SProgUniVar& sProgVar, Fai val);
		MaterialInputVariable(const SProgUniVar& sProgVar,
			const char* texFilename);
		/// @}

		~MaterialInputVariable();

		/// @name Accessors
		/// @{
		float getFloat() const;
		const Vec2& getVec2() const;
		const Vec3& getVec3() const;
		const Vec4& getVec4() const;
		Fai getFai() const;
		const Texture& getTexture() const;

		GETTER_R(SProgUniVar, sProgVar, getShaderProgramUniformVariable)
		GETTER_R_BY_VAL(Type, type, getType)
		/// @}

	private:
		const SProgUniVar& sProgVar; ///< The shader program's uniform
		Type type;

		/// @name Data
		/// @{
		float scalar;
		Vec2 vec2;
		Vec3 vec3;
		Vec4 vec4;
		Fai fai;
		RsrcPtr<Texture> texture;
		/// @}
};

#endif
