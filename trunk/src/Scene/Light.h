/*
LIGHTING MODEL

Final intensity:                If = Ia + Id + Is
Ambient intensity:              Ia = Al * Am
Ambient intensity of light:     Al
Ambient intensity of material:  Am
Defuse intensity:               Id = Dl * Dm * LambertTerm
Defuse intensity of light:      Dl
Defuse intensity of material:   Dm
LambertTerm:                    max(Normal dot Light, 0.0)
Specular intensity:             Is = Sm x Sl x pow(max(R dot E, 0.0), f)
Specular intensity of light:    Sl
Specular intensity of material: Sm
*/

#ifndef _LIGHT_H_
#define _LIGHT_H_

#include "Common.h"
#include "Texture.h"
#include "SceneNode.h"
#include "Camera.h"
#include "RsrcPtr.h"
#include "LightData.h"


/// Light scene node (Abstract)
class Light: public SceneNode
{
	public:
		enum LightType
		{
			LT_POINT,
			LT_SPOT
		};

	PROPERTY_R(LightType, type, getType) ///< Light type

	public:
		RsrcPtr<LightData> lightData;
	
		Light(LightType type_);
		void render();

	private:

};


/// PointLight scene node
class PointLight: public Light
{
	PROPERTY_RW(float, radius, setRadius, getRadius)

	public:
		PointLight();
		void init(const char*);
};


/// SpotLight scene node
class SpotLight: public Light
{
	public:
		Camera* camera;
		bool castsShadow;

		SpotLight();
		float getDistance() const;
		void setDistance(float d);
		void init(const char*);
};


//======================================================================================================================
// Inlines                                                                                                             =
//======================================================================================================================

inline Light::Light(LightType type_):
	SceneNode(SNT_LIGHT),
	type(type_)
{}


inline PointLight::PointLight():
	Light(LT_POINT)
{}


inline SpotLight::SpotLight():
	Light(LT_SPOT),
	camera(new Camera(this)),
	castsShadow(false)
{}


inline float SpotLight::getDistance() const
{
	return camera->getZFar();
}


inline void SpotLight::setDistance(float d)
{
	camera->setZFar(d);
}

#endif
