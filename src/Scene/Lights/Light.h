#ifndef LIGHT_H
#define LIGHT_H

#include "Texture.h"
#include "SceneNode.h"
#include "RsrcPtr.h"
#include "LightRsrc.h"
#include "VisibilityInfo.h"


/// Light scene node. It can be spot or point
////
/// Explaining the lighting model:
/// @code
/// Final intensity:                If = Ia + Id + Is
/// Ambient intensity:              Ia = Al * Am
/// Ambient intensity of light:     Al
/// Ambient intensity of material:  Am
/// Defuse intensity:               Id = Dl * Dm * LambertTerm
/// Defuse intensity of light:      Dl
/// Defuse intensity of material:   Dm
/// LambertTerm:                    max(Normal dot Light, 0.0)
/// Specular intensity:             Is = Sm * Sl * pow(max(R dot E, 0.0), f)
/// Specular intensity of light:    Sl
/// Specular intensity of material: Sm
/// @endcode
class Light: public SceneNode, public VisibilityInfo
{
	public:
		enum LightType
		{
			LT_POINT,
			LT_SPOT
		};

		Light(LightType type, bool compoundFlag, SceneNode* parent = NULL);
		~Light() {}

		/// @name Accessors
		/// @{
		GETTER_R(LightType, type, getType)

		GETTER_SETTER(Vec3, diffuseCol, getDiffuseCol, setDiffuseCol)
		GETTER_SETTER(Vec3, specularCol, getSpecularCol, setSpecularCol)
		GETTER_SETTER_BY_VAL(bool, castsShadowFlag, castsShadow, setCastsShadow)
		/// @}

		void init(const char* filename);

		void moveUpdate() {}
		void frameUpdate(float /*prevUpdateTime*/, float /*crntTime*/) {}

	protected:
		RsrcPtr<LightRsrc> lightData;
		Vec3 diffuseCol; ///< Diffuse color
		Vec3 specularCol; ///< Specular color
		bool castsShadowFlag; ///< Casts shadow

	private:
		LightType type; ///< Light type
};


inline Light::Light(LightType type_, bool compoundFlag, SceneNode* parent):
	SceneNode(SNT_LIGHT, compoundFlag, parent),
	type(type_)
{}


#endif
