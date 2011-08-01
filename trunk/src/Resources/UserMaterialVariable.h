#ifndef USER_MATERIAL_VARIABLE_H
#define USER_MATERIAL_VARIABLE_H

#include "MaterialVariable.h"
#include "Math/Math.h"
#include "RsrcPtr.h"


class Texture;
class UniformShaderProgramVariable;


/// XXX
class UserMaterialVariable: public MaterialVariable
{
	public:
		/// @name Constructors
		/// @{
		UserMaterialVariable(const UniformShaderProgramVariable* cpUni,
			const UniformShaderProgramVariable* dpUni, float val);
		UserMaterialVariable(const UniformShaderProgramVariable* cpUni,
			const UniformShaderProgramVariable* dpUni, const Vec2& val);
		UserMaterialVariable(const UniformShaderProgramVariable* cpUni,
			const UniformShaderProgramVariable* dpUni, const Vec3& val);
		UserMaterialVariable(const UniformShaderProgramVariable* cpUni,
			const UniformShaderProgramVariable* dpUni, const Vec4& val);
		UserMaterialVariable(const UniformShaderProgramVariable* cpUni,
			const UniformShaderProgramVariable* dpUni, const char* texFilename);
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
		/// XXX
		struct Data
		{
			float scalar;
			Vec2 vec2;
			Vec3 vec3;
			Vec4 vec4;
			RsrcPtr<Texture> texture;
		};

		Data data;
};


#endif
