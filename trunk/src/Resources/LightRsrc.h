#ifndef LIGHT_PROPS_H
#define LIGHT_PROPS_H

#include "Math.h"
#include "RsrcPtr.h"
#include "Texture.h"
#include "Properties.h"


/// Properties common for all lights
struct LightProps
{
	/// @name Common light properties
	/// @{
	Vec3 diffuseCol;
	Vec3 specularCol;
	bool castsShadowFlag; ///< Currently only for spot lights
	/// @}

	/// @name Point light properties
	/// @{
	float radius; ///< Sphere radius
	/// @}

	/// @name Spot light properties
	/// @{
	float distance; ///< AKA camera's zFar
	float fovX;
	float fovY;
	/// @}
};


/// Light properties Resource
class LightRsrc: private LightProps
{
	public:
		enum LightType
		{
			LT_POINT,
			LT_SPOT,
			LT_NUM
		};

		LightRsrc();
		~LightRsrc() {}

		/// @name Accessors
		/// @{
		GETTER_R(Vec3, diffuseCol, getDiffuseCol)
		GETTER_R(Vec3, specularCol, getSpecularCol)
		GETTER_R_BY_VAL(bool, castsShadowFlag, castsShadow)
		GETTER_R_BY_VAL(LightType, type, getType)

		GETTER_R_BY_VAL(float, radius, getRadius)

		GETTER_R_BY_VAL(float, distance, getDistance)
		GETTER_R_BY_VAL(float, fovX, getFovX)
		GETTER_R_BY_VAL(float, fovY, getFovY)
		const Texture& getTexture() const;
		/// @}

		void load(const char* filename);

	private:
		LightType type;
		RsrcPtr<Texture> texture;
};


inline const Texture& LightRsrc::getTexture() const
{
	ASSERT(texture.get() != NULL);
	return *texture;
}


#endif
