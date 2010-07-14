#ifndef _LIGHTPROPS_H_
#define _LIGHTPROPS_H_

#include "Common.h"
#include "Resource.h"
#include "Math.h"
#include "RsrcPtr.h"
#include "Texture.h"


/**
 * Light properties @ref Resource
 */
class LightProps: public Resource
{
	PROPERTY_R(Vec3, diffuseCol, getDiffuseColor)
	PROPERTY_R(Vec3, specularCol, getSpecularColor)
	PROPERTY_R(float, radius, getRadius) ///< For point lights
	PROPERTY_R(bool, castsShadow_, castsShadow) ///< For spot lights
	PROPERTY_R(float, distance, getDistance) ///< For spot lights. A.K.A.: camera's zFar
	PROPERTY_R(float, fovX, getFovX) ///< For spot lights
	PROPERTY_R(float, fovY, getFovY) ///< For spot lights
		
	public:
		LightProps();
		virtual ~LightProps() {}
		bool load(const char* filename);
		void unload() {}
		const Texture* getTexture() const;

	private:
		RsrcPtr<Texture> texture; ///< For spot lights
};


inline LightProps::LightProps():
	Resource(RT_LIGHT_PROPS),
	diffuseCol(0.5),
	specularCol(0.5),
	radius(1.0),
	castsShadow_(false),
	distance(3.0),
	fovX(M::PI/4.0),
	fovY(M::PI/4.0)
{}


inline const Texture* LightProps::getTexture() const
{
	DEBUG_ERR(texture.get() == NULL);
	return texture.get();
}

#endif
