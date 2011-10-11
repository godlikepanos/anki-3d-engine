#ifndef ANKI_SCENE_SPOT_LIGHT_H
#define ANKI_SCENE_SPOT_LIGHT_H

#include "anki/scene/Light.h"
#include "anki/scene/PerspectiveCamera.h"


/// Spot light
class SpotLight: public Light
{
	public:
		SpotLight(Scene& scene, ulong flags, SceneNode* parent);
		~SpotLight()
		{}

		void init(const char* filename);

		/// @name Accessors
		/// @{
		float getDistance() const
		{
			return camera->getZFar();
		}
		void setDistance(float d)
		{
			camera->setZFar(d);
		}

		float getFovX() const
		{
			return camera->getFovX();
		}
		void setFovX(float f)
		{
			camera->setFovX(f);
		}

		float getFovY() const
		{
			return camera->getFovY();
		}
		void setFovY(float f)
		{
			camera->setFovY(f);
		}

		const Texture& getTexture() const
		{
			return lightData->getTexture();
		}

		const Camera& getCamera() const
		{
			return *camera;
		}
		Camera& getCamera()
		{
			return *camera;
		}
		/// @}

		const CollisionShape*
			getVisibilityCollisionShapeWorldSpace() const
		{
			return camera->getVisibilityCollisionShapeWorldSpace();
		}

	private:
		PerspectiveCamera* camera;
};


inline SpotLight::SpotLight(Scene& scene, ulong flags, SceneNode* parent)
:	Light(LT_SPOT, scene, flags, parent)
{}


#endif
