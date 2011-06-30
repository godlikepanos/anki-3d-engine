#ifndef LIGHT_PROPS_H
#define LIGHT_PROPS_H

#include "Math/Math.h"
#include "RsrcPtr.h"
#include "Resources/Texture.h"
#include "Util/Accessors.h"


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
	float fovX; ///< For perspective camera
	float fovY; ///< For perspective camera
	float width; ///< For orthographic camera
	float height; ///< For orthographic camera
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

		enum SpotLightCameraType
		{
			SLCT_PERSPECTIVE,
			SLCT_ORTHOGRAPHIC
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
		GETTER_R_BY_VAL(float, width, getWidth)
		GETTER_R_BY_VAL(float, height, getHeight)
		const Texture& getTexture() const;
		/// @}

		void load(const char* filename);

	private:
		LightType type;
		SpotLightCameraType spotLightCameraType;
		RsrcPtr<Texture> texture;
};


inline const Texture& LightRsrc::getTexture() const
{
	ASSERT(texture.get() != NULL);
	return *texture;
}


#endif
