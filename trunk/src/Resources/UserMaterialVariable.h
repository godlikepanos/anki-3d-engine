#ifndef USER_MATERIAL_VARIABLE_H
#define USER_MATERIAL_VARIABLE_H

#include "MaterialVariable.h"
#include "Math/Math.h"
#include "RsrcPtr.h"


class Texture;
class SProgUniVar;


/// XXX
class UserMaterialVariable: public MaterialVariable
{
	public:
		/// XXX
		struct Data
		{
			float scalar;
			Vec2 vec2;
			Vec3 vec3;
			Vec4 vec4;
			RsrcPtr<Texture> texture;
		};

		/// @name Constructors
		/// @{
		UserMaterialVariable(const SProgUniVar* cpSProgUniVar,
			const SProgUniVar* dpSProgUniVar, float val);
		UserMaterialVariable(const SProgUniVar* cpSProgUniVar,
			const SProgUniVar* dpSProgUniVar, const Vec2& val);
		UserMaterialVariable(const SProgUniVar* cpSProgUniVar,
			const SProgUniVar* dpSProgUniVar, const Vec3& val);
		UserMaterialVariable(const SProgUniVar* cpSProgUniVar,
			const SProgUniVar* dpSProgUniVar, const Vec4& val);
		UserMaterialVariable(const SProgUniVar* cpSProgUniVar,
			const SProgUniVar* dpSProgUniVar, const char* texFilename);
		/// @}

		/// @name Accessors
		/// @{
		float getFloat() const;
		const Vec2& getVec2() const;
		const Vec3& getVec3() const;
		const Vec4& getVec4() const;
		const Texture& getTexture() const;
		/// @}

	private:
		Data data;
};


#endif
