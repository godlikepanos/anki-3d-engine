#ifndef LIGHT_PROPS_H
#define LIGHT_PROPS_H

#include "m/Math.h"
#include "RsrcPtr.h"
#include "rsrc/Texture.h"


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
		~LightRsrc()
		{}

		/// @name Accessors
		/// @{
		const Vec3& getDiffuseColor() const
		{
			return diffuseCol;
		}

		const Vec3& getSpecularColor() const
		{
			return specularCol;
		}

		bool getCastShadow() const
		{
			return castsShadowFlag;
		}

		LightType getType() const
		{
			return type;
		}

		float getRadius() const
		{
			return radius;
		}

		float getDistance() const
		{
			return distance;
		}

		float getFovX() const
		{
			return fovX;
		}

		float getFovY() const
		{
			return fovY;
		}

		float getWidth() const
		{
			return width;
		}

		float getHeight() const
		{
			return height;
		}

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
