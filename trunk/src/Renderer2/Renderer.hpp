#ifndef _RENDERER_HPP_
#define _RENDERER_HPP_

#include "Common.h"
#include "Math.h"

class Camera;
class PointLight;
class SpotLight;


/**
 * @todo write
 *
 * @note Because of the many members of the class there is a naming convention that helps
 * - MS: material stage
 * - IS: illumination stage
 * - SM: shadowmapping
 * - AP: ambient pass
 * - PLS: point light pass
 * - SLS: spot light pass
 * - PPS: preprocessing stage
 * - HDR: high dynamic range lighting
 * - SSAO: screen space ambient occlusion
 * - LScat: light scattering
 * - FAI: frame buffer attachable image
 */
class Renderer
{
	public:
		// quality
		uint  width; ///< width of the rendering. Dont confuse with the window width
		uint  height; ///< height of the rendering. Dont confuse with the window width
		float renderingQuality; ///< The global rendering quality of the raster image. From 0.0(low) to 1.0(high)
		int   screenshotJpegQuality; ///< The quality of the JPEG screenshots. From 0 to 100
		// texture stuff
		bool textureCompression; ///< Used in Texture::load to enable texture compression. Decreases video memory usage
		int  maxTextureUnits; ///< Used in Texture::bind so we wont bind in a nonexistent texture unit. Readonly
		bool mipmaping; ///< Used in Texture::load. Enables mipmaping increases video memory usage
		int  maxAnisotropy; ///< Max texture anisotropy. Used in Texture::load
		// other
		int   maxColorAtachments; ///< Max color attachments a FBO can accept
		uint  framesNum;
		float aspectRatio;
		// matrices & viewing
		Camera* crntCamera;
		Mat4 modelViewMat; ///< This changes once for every mesh rendering
		Mat4 projectionMat; ///< This changes once every frame
		Mat4 modelViewProjectionMat; ///< This changes just like @ref modelViewMat
		Mat3 normalMat; ///< The rotation part of modelViewMat

	//===================================================================================================================================
	// Material Stage                                                                                                                   =
	//===================================================================================================================================
	private:
		Fbo msFbo;

		void initMs();
		void runMs();

	public:
		Texture normalMsFai;
		Texture diffuseMsFai;
		Texture specularMsFai;
		Texture depthMsFai;

	//===================================================================================================================================
	// Illumination Stage                                                                                                               =
	//===================================================================================================================================
	private:
		Fbo isFbo;
		uint stencilRb; ///< Illumination stage stencil buffer
		ShaderProg apIsSProg; ///< Illumination stage ambient pass shader program
		ShaderProg plIsSProg; ///< Illumination stage point light shader program
		ShaderProg slIsSProg; ///< Illumination stage spot light w/o shadow shader program
		ShaderProg slSIsSProg; ///< Illumination stage spot light w/ shadow shader program
		Vec3 viewVectors[4];
		Vec2 planes;
		static float sMOUvSCoords []; ///< Illumination stage stencil masking optimizations UV sphere vertex coords
		static uint  sMOUvSVboId; ///< Illumination stage stencil masking optimizations UV sphere VBO id

		static void initSMOUvS(); ///< Init the illumination stage stencil masking optimizations uv sphere (eg create the @ref sMOUvSVboId VBO)
		void renderSMOUvS( const PointLight& light ); ///< Render the illumination stage stencil masking optimizations uv sphere
		void calcViewVector(); ///< Calc the view vector that we will use inside the shader to calculate the frag pos in view space
		void calcPlanes(); ///< Calc the planes that we will use inside the shader to calculate the frag pos in view space
		void setStencilMask( const PointLight& light );
		void setStencilMask( const SpotLight& light );
		void ambientPass( const Vec3& color );
		void pointLightPass( const PointLight& light );
		void spotLightPass( const SpotLight& light );
		void initIsFbo();
		void initIs();
		void runIs();

	public:
		Texture isFai;

	//===================================================================================================================================
	// Illumination Stage Shadowmapping                                                                                                 =
	//===================================================================================================================================
	private:
		Fbo smFbo; ///< Illumination stage shadowmapping FBO
		Texture shadowMap;

		void initSm();

	public:
		bool smPcfEnabled;
		bool smBilinearEnabled;
		int  smResolution;
};

#endif







