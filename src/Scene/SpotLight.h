#ifndef SPOT_LIGHT_H
#define SPOT_LIGHT_H

#include "Light.h"


/// Spot light
class SpotLight: public Light
{
	public:
		SpotLight(SceneNode* parent = NULL): Light(LT_SPOT, parent) {}
		~SpotLight() {}
		void init(const char* filename);

		/// @name Setters & getters
		/// @{
		float getDistance() const {return camera->getZFar();}
		void setDistance(float d) {camera->setZFar(d);}
		float getFovX() const {return camera->getFovX();}
		void setFovX(float f) {camera->setFovX(f);}
		float getFovY() const {return camera->getFovY();}
		void setFovY(float f) {camera->setFovY(f);}
		const Texture& getTexture() const {return lightData->getTexture();}
		const Camera& getCamera() const {return *camera;}
		Camera& getCamera() {return *camera;}
		/// @}

	private:
		Camera* camera;
};


#endif
