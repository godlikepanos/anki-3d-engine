#ifndef MATERIAL_PROPERTIES_H
#define MATERIAL_PROPERTIES_H

#include <GL/glew.h>


/// Contains a few properties that other classes may use
struct MaterialProperties
{
	public:
		/// @name Accessors
		/// @{
		bool getCastShadow() const {return castsShadowFlag;}
		bool getRenderInBledingStage() const {return renderInBlendingStageFlag;}
		int getBlendingSFactor() const {return blendingSfactor;}
		int getBlendingDFactor() const {return blendingDfactor;}
		bool getDepthTesting() const {return depthTesting;}
		bool getWireframe() const {return wireframe;}
		/// @}

		/// Check if blending is enabled
		bool isBlendingEnabled() const
			{return blendingSfactor != GL_ONE || blendingDfactor != GL_ZERO;}

	protected:
		/// Used in depth passes of shadowmapping and not in other depth passes
		/// like EarlyZ
		bool castsShadowFlag;
		/// The entities with blending are being rendered in blending stage and
		/// those without in material stage
		bool renderInBlendingStageFlag;
		int blendingSfactor; ///< Default GL_ONE
		int blendingDfactor; ///< Default GL_ZERO
		bool depthTesting;
		bool wireframe;
};


#endif
