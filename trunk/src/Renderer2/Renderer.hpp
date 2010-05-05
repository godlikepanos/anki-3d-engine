#ifndef _RENDERER_HPP_
#define _RENDERER_HPP_

#include "Common.h"
#include "Math.h"

class Camera;
class PointLight;
class SpotLight;
class Texture;


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
	//===================================================================================================================================
	// Rendering Stages                                                                                                                 =
	//===================================================================================================================================
	public:

		/**
		 * Rendering stage base class
		 */
		class RenderingStage
		{
			protected:
				Renderer& r;
			public:
				RenderingStage( Renderer& r_ ): r(r_) {}
		};


		/**
		 * Material stage
		 */
		class Ms: public RenderingStage
		{
			private:
				Fbo fbo;

				void init();
				void run();

			public:
				Texture normalFai;
				Texture diffuseFai;
				Texture specularFai;
				Texture depthFai;

				Ms( Renderer& r_ ): RenderingStage(r_) {}
		}; // end Ms


		/**
		 * Illumination stage
		 */
		class Is: public RenderingStage
		{
			public:
				/**
				 * Shadowmapping sub-stage
				 */
				class Sm: public RenderingStage
				{
					private:
						Fbo fbo; ///< Illumination stage shadowmapping FBO
						Texture shadowMap;

						void init();
						void run( const Camera& cam );

					public:
						bool pcfEnabled;
						bool bilinearEnabled;
						int  mapRez;

						Sm( Renderer& r_ ): RenderingStage(r_) {}
				};

			private:
				Fbo fbo;
				uint stencilRb; ///< Illumination stage stencil buffer
				ShaderProg apSProg; ///< Illumination stage ambient pass shader program
				ShaderProg plSProg; ///< Illumination stage point light shader program
				ShaderProg slSProg; ///< Illumination stage spot light w/o shadow shader program
				ShaderProg slSSProg; ///< Illumination stage spot light w/ shadow shader program
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
				void initFbo();
				void init();
				void run();

			public:
				Texture fai;
				Sm sm;

				Is( Renderer& r_ ): RenderingStage(r_), Sm(r) {}
		}; // end Is


		/**
		 * Post-processing stage
		 */
		class Pps: public RenderingStage
		{
			public:
				/**
				 * High dynamic range lighting
				 */
				class Hdr: public RenderingStage
				{
					private:
						Fbo pass0Fbo, pass1Fbo, pass2Fbo;
						float renderingQuality;
						uint w, h;
						ShaderProg pass0SProg, pass1SProg, pass2SProg;

						void initFbos( Fbo& fbo, Texture& fai, int internalFormat );
						void init();
						void run();

					public:
						Texture pass0Fai; ///< Vertical blur pass
						Texture pass1Fai; ///< @ref pass0Fai with the horizontal blur
						Texture fai; ///< The final FAI
						bool enabled;

						Hdr( Renderer& r_ ): RenderingStage(r_) {}
				}; // end Hdr

				/**
				 * Screen space ambient occlusion
				 */
				class Ssao: public RenderingStage
				{
					private:
						Fbo pass0Fbo, pass1Fbo, pass2Fbo;
						uint w, h;
						uint bw, bh;
						ShaderProg ssaoSProg, blurSProg, blurSProg2;
						Texture* noiseMap;

						void initBlurFbos();
						void init();
						void run();

					public:
						bool enabled;
						float renderingQuality;
						float bluringQuality;
						Texture pass0Fai, pass1Fai, fai;

						Ssao( Renderer& r_ ): RenderingStage(r_) {}
				}; // end Ssao

				/**
				 * Light scattering
				 */
				class Lscatt: public RenderingStage
				{
					private:
						Fbo fbo;
						ShaderProg sProg;
						int msDepthFaiUniLoc;
						int isFaiUniLoc;

						void init();
						void run();

					public:
						float renderingQuality;
						bool enabled;
						Texture fai;

						Lscatt( Renderer& r_ ): RenderingStage(r_) {}
				}; // end Lscatt

			private:
				Fbo fbo;
				ShaderProg sProg;
				struct
				{
					int isFai;
					int ppsSsaoFai;
					int msNormalFai;
					int hdrFai;
					int lscattFai;
				} uniLocs;

				void init();
				void run();

			public:
				Texture fai;

				Hdr hdr;
				Ssao ssao;
				Lscatt lscatt;

				Pps( Renderer& r_ ): RenderingStage(r_), hdr(r), ssao(r), lscatt(r) {}
		}; // end Pps


		/**
		 * Blending stage
		 */
		class Bs: public RenderingStage
		{
			private:
			public:
				Bs( Renderer& r_ ): RenderingStage(r_) {}
		}; // end Bs


		// define the stages
		Ms  ms;
		Is  is;
		Pps pps;
		Bs  bs;

	//===================================================================================================================================
	//                                                                                                                                  =
	//===================================================================================================================================
	public:
		// quality
		uint  width; ///< width of the rendering. Dont confuse with the window width
		uint  height; ///< height of the rendering. Dont confuse with the window width
		float renderingQuality; ///< The global rendering quality of the raster image. From 0.0(low) to 1.0(high)
		int   screenshotJpegQuality; ///< The quality of the JPEG screenshots. From 0 to 100
		// texture stuff
		bool textureCompression; ///< Used in Texture::load to enable texture compression. Decreases video memory usage
		int  maxTextureUnits; ///< Used in Texture::bind so we wont bind in a nonexistent texture unit. Readonly
		bool mipmaping; ///< Used in Texture::load. Enables mipmapping increases video memory usage
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

};

#endif







