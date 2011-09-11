#ifndef LIGHT_H
#define LIGHT_H

#include "rsrc/Texture.h"
#include "SceneNode.h"
#include "rsrc/RsrcPtr.h"
#include "rsrc/LightRsrc.h"
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
		Light(ClassId cid, Scene& scene, ulong flags, SceneNode* parent);
		~Light();

		static bool classof(const SceneNode* x);

		/// @name Accessors
		/// @{
		GETTER_SETTER(Vec3, diffuseCol, getDiffuseCol, setDiffuseCol)
		GETTER_SETTER(Vec3, specularCol, getSpecularCol, setSpecularCol)
		GETTER_SETTER_BY_VAL(bool, castsShadowFlag, castsShadow, setCastsShadow)
		/// @}

		void init(const char* filename);

	protected:
		RsrcPtr<LightRsrc> lightData;
		Vec3 diffuseCol; ///< Diffuse color
		Vec3 specularCol; ///< Specular color
		bool castsShadowFlag; ///< Casts shadow
};


inline Light::Light(ClassId cid, Scene& scene, ulong flags, SceneNode* parent)
:	SceneNode(cid, scene, flags, parent)
{}


inline bool Light::classof(const SceneNode* x)
{
	return x->getClassId() == CID_LIGHT ||
		x->getClassId() == CID_POINT_LIGHT ||
		x->getClassId() == CID_SPOT_LIGHT;
}


#endif
