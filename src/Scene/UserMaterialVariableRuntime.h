#ifndef USER_MATERIAL_VARIABLE_RUNTIME_H
#define USER_MATERIAL_VARIABLE_RUNTIME_H

#include "Util/Accessors.h"
#include "Math/Math.h"


class UserMaterialVariable;
class Texture;


/// XXX
class UserMaterialVariableRuntime
{
	public:
		/// Constructor
		UserMaterialVariableRuntime(const UserMaterialVariable& umv);

		/// @name Accessors
		/// @{
		GETTER_R(UserMaterialVariable, umv, getUserMaterialVariable)

		GETTER_SETTER_BY_VAL(float, data.scalar, getFloat, setFloat)
		GETTER_SETTER(Vec2, data.vec2, getVec2, setVec2)
		GETTER_SETTER(Vec3, data.vec3, getVec3, setVec3)
		GETTER_SETTER(Vec4, data.vec4, getVec4, setVec4)
		const Texture& getTexture() const {return *data.tex;}
		/// @}

	private:
		/// XXX
		struct Data
		{
			float scalar;
			Vec2 vec2;
			Vec3 vec3;
			Vec4 vec4;
			const Texture* tex;
		};

		const UserMaterialVariable& umv; ///< Know the resource
		Data data;
};


#endif
