#ifndef ANKI_SCENE_LIGHT_H
#define ANKI_SCENE_LIGHT_H

#include "anki/resource/Texture.h"
#include "anki/scene/SceneNode.h"
#include "anki/resource/Resource.h"
#include "anki/resource/LightRsrc.h"
#include "anki/scene/VisibilityInfo.h"
#include "anki/scene/Renderable.h"


namespace anki {


/// Light scene node. It can be spot or point
////
/// Explaining the lighting model:
/// @code
/// Final intensity:                If = Ia + Id + Is
/// Ambient intensity:              Ia = Al * Am
/// Ambient intensity of light:     Al
/// Ambient intensity of material:  Am
/// Diffuse intensity:              Id = Dl * Dm * LambertTerm
/// Diffuse intensity of light:     Dl
/// Diffuse intensity of material:  Dm
/// LambertTerm:                    max(Normal dot Light, 0.0)
/// Specular intensity:             Is = Sm * Sl * pow(max(R dot E, 0.0), f)
/// Specular intensity of light:    Sl
/// Specular intensity of material: Sm
/// @endcode
class Light: public SceneNode, public VisibilityInfo/*, public Renderable*/
{
	public:
		enum LightType
		{
			LT_POINT,
			LT_SPOT
		};

		Light(LightType t, Scene& scene, ulong flags, SceneNode* parent)
			: SceneNode(SNT_LIGHT, scene, flags, parent), type(t)
		{}

		virtual ~Light();

		/// @name Accessors
		/// @{
		LightType getLightType() const {return type;}

		const Vec3& getDiffuseColor() const {return diffuseCol;}
		Vec3& getDiffuseColor() {return diffuseCol;}
		void setDiffuseColor(const Vec3& x) {diffuseCol = x;}

		const Vec3& getSpecularColor() const {return specularCol;}
		Vec3& getSpecularColor() {return specularCol;}
		void setSpecularColor(const Vec3& x) {specularCol = x;}

		bool getCastShadow() const {return castsShadowFlag;}
		bool& getCastShadow() {return castsShadowFlag;}
		void setCastShadow(bool x) {castsShadowFlag = x;}
		/// @}

		/// Implements Renderable::getMaterialRuntime
		/*MaterialRuntime& getMaterialRuntime()
		{
			return *mtlr;
		}*/

		void init(const char* filename);

	protected:
		//boost::scoped_ptr<MaterialRuntime> mtlr;
		LightRsrcResourcePointer lightData;
		Vec3 diffuseCol; ///< Diffuse color
		Vec3 specularCol; ///< Specular color
		bool castsShadowFlag; ///< Casts shadow

	private:
		LightType type;
};


} // end namespace


#endif
