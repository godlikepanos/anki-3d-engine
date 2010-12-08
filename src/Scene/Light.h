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

#ifndef LIGHT_H
#define LIGHT_H

#include "Texture.h"
#include "SceneNode.h"
#include "Camera.h"
#include "RsrcPtr.h"
#include "LightData.h"


/// Light scene node. It can be spot or point
class Light: public SceneNode
{
	public:
		enum LightType
		{
			LT_POINT,
			LT_SPOT
		};

	PROPERTY_R(LightType, type, getType) ///< Light type

	/// @name Copies of some of the resource properties. The others are camera properties or not changeable
	/// @{
	PROPERTY_RW(Vec3, diffuseCol, getDiffuseCol, setDiffuseCol) ///< Diffuse color
	PROPERTY_RW(Vec3, specularCol, getSpecularCol, setSpecularCol) ///< Specular color
	PROPERTY_RW(bool, castsShadow_, castsShadow, setCastsShadow) ///< Casts shadow
	/// @}

	public:
		RsrcPtr<LightData> lightData;
	
		Light(LightType type, SceneNode* parent = NULL);
		~Light() {}
		void init(const char* filename);
};


inline Light::Light(LightType type_, SceneNode* parent):
	SceneNode(SNT_LIGHT, parent),
	type(type_)
{}


#endif
