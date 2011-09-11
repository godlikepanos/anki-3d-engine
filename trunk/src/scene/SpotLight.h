#ifndef SPOT_LIGHT_H
#define SPOT_LIGHT_H

#include "Light.h"
//#include "PerspectiveCamera.h"


/// Spot light
class SpotLight: public Light
{
	public:
		SpotLight(Scene& scene, ulong flags, SceneNode* parent);
		~SpotLight() {}

		static bool classof(const SceneNode* x);

		void init(const char* filename);

		/// @name Accessors
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
		PerspectiveCamera* camera;
};


inline SpotLight::SpotLight(Scene& scene, ulong flags, SceneNode* parent)
:	Light(CID_SPOT_LIGHT, scene, flags, parent)
{}


bool SpotLight::classof(const SceneNode* x)
{
	return x->getClassId() == CID_LIGHT &&
		static_cast<const Light*>(x)->getClassId() == CID_SPOT_LIGHT;
}


#endif
