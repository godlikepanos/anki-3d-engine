#ifndef LIGHT_PROPS_H
#define LIGHT_PROPS_H

#include "Common.h"
#include "Resource.h"
#include "Math.h"
#include "RsrcPtr.h"
#include "Texture.h"


/// Light properties Resource
class LightData: public Resource
{
	public:
		enum LightType
		{
			LT_POINT,
			LT_SPOT,
			LT_NUM
		};

	/// @name Common light properties
	/// @{
	PROPERTY_R(Vec3, diffuseCol, getDiffuseCol)
	PROPERTY_R(Vec3, specularCol, getSpecularCol)
	PROPERTY_R(bool, castsShadow_, castsShadow) ///< Currently only for spot lights
	PROPERTY_R(LightType, type, getType)
	/// @}

	/// @name Point light properties
	/// @{
	PROPERTY_R(float, radius, getRadius) ///< Sphere radius
	/// @}

	/// @name Spot light properties
	/// @{
	PROPERTY_R(float, distance, getDistance) ///< AKA camera's zFar
	PROPERTY_R(float, fovX, getFovX)
	PROPERTY_R(float, fovY, getFovY)
	/// @}
		
	public:
		LightData();
		~LightData() {}
		bool load(const char* filename);
		const Texture& getTexture() const;

	private:
		/// @name Spot light properties
		/// @{
		RsrcPtr<Texture> texture;
		/// @}
};


inline const Texture& LightData::getTexture() const
{
	DEBUG_ERR(texture.get() == NULL);
	return *texture;
}


#endif
