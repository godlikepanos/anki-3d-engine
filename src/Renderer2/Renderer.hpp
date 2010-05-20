#ifndef _RENDERER_HPP_
#define _RENDERER_HPP_

#include "Common.h"
#include "Math.h"
#include "Fbo.h"
#include "Texture.h"
#include "ShaderProg.h"

class Camera;
class PointLight;
class SpotLight;


/**
 * The renderer class
 *
 * It is a class and not a namespace because we may need external renderers for security cameras for example
 */
class Renderer
{
	//===================================================================================================================================
	// The rendering stages                                                                                                             =
	//===================================================================================================================================
	public:
		/**
		 * Rendering stage
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
		class Ms: private RenderingStage
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

				Ms( Renderer& r_ ): RenderingStage( r_ ) {}
		}; // end Ms

		/**
		 * Illumination stage
		 */
		class Is: private RenderingStage
		{
			public:
				/**
				 * Shadowmapping sub-stage
				 */
				class Sm: private RenderingStage
				{
					friend class Is;

					private:
						Fbo fbo; ///< Illumination stage shadowmapping FBO
						Texture shadowMap;

						void init();

						/**
						 * Render the scene only with depth and store the result in the shadowMap
						 * @param cam The light camera
						 */
						void run( const Camera& cam );

					public:
						bool pcfEnabled;
						bool bilinearEnabled;
						int  resolution; ///< Shadowmap resolution. The higher the more quality

						Sm( Renderer& r_ ): RenderingStage( r_ ) {}
				}; // end Sm

			private:
				Fbo fbo;
				uint stencilRb; ///< Illumination stage stencil buffer
				// shader stuff
				/// Illumination stage ambient pass shader program
				class AmbientShaderProg: public ShaderProg
				{
					public:
						struct
						{
							int ambientCol, sceneColMap;
						}uniLocs;
				};
				/// Illumination stage light pass shader program
				class LightShaderProg: public ShaderProg
				{
					public:
						struct
						{
							int msNormalFai, msDiffuseFai, msSpecularFai, msDepthFai, planes, lightPos, lightInvRadius, lightDiffuseCol, lightSpecularCol, lightTex, texProjectionMat, shadowMap;
						}uniLocs;
				};
				AmbientShaderProg ambientPassSProg; ///< Illumination stage ambient pass shader program
				LightShaderProg pointLightSProg; ///< Illumination stage point light shader program
				LightShaderProg spotLightNoShadowSProg; ///< Illumination stage spot light w/o shadow shader program
				LightShaderProg spotLightShadowSProg; ///< Illumination stage spot light w/ shadow shader program
				// other
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

				Is( Renderer& r_ ): RenderingStage( r_ ), sm(r) {}
		}; // end Is


		/**
		 * Post-processing stage
		 */
		class Pps: private RenderingStage
		{
			public:
				/**
				 * High dynamic range lighting stage
				 */
				class Hdr: private RenderingStage
				{
					private:
						Fbo pass0Fbo, pass1Fbo, pass2Fbo;
						ShaderProg pass0SProg, pass1SProg, pass2SProg;
						struct
						{
							struct
							{
								int fai;
							} pass0SProg;
							struct
							{
								int fai;
							} pass1SProg;
							struct
							{
								int fai;
							} pass2SProg;
						} uniLocs;

						void initFbos( Fbo& fbo, Texture& fai, int internalFormat );
						void init();
						void run();

					public:
						bool enabled;
						float renderingQuality;
						Texture pass0Fai; ///< Vertical blur pass
						Texture pass1Fai; ///< pass0Fai with the horizontal blur
						Texture fai; ///< The final FAI

						Hdr( Renderer& r_ ): RenderingStage(r_) {}
				}; // end Hrd

				/**
				 * Screen space ambient occlusion stage
				 *
				 * Three passes:
				 * - Calc ssao factor
				 * - Blur vertically
				 * - Blur horizontally
				 */
				class Saao: private RenderingStage
				{
					private:
						Fbo pass0Fbo, pass1Fbo, pass2Fbo;
						uint width, height, bwidth, bheight;
						Texture* noiseMap;
						struct
						{
							struct
							{
								int camerarange, msDepthFai, noiseMap, msNormalFai;
							} pass0SProg;
							struct
							{
								int fai;
							} pass1SProg;
							struct
							{
								int fai;
							} pass2SProg;
						} uniLocs;

						void initBlurFbo( Fbo& fbo, Texture& fai );
						void init();
						void run();

					public:
						float renderingQuality;
						float bluringQuality;
						Texture pass0Fai, pass1Fai, fai /** The final FAI */;
						ShaderProg ssaoSProg, blurSProg, blurSProg2;

						Saao( Renderer& r_ ): RenderingStage(r_) {}
				}; // end Ssao

				private:
					class PpsShaderProg: public ShaderProg
					{
						public:
							struct
							{
								int isFai, ppsSsaoFai, msNormalFai, hdrFai, lscattFai;
							} uniLocs;
					};
					PpsShaderProg sProg;
					Fbo fbo;

					void init();
					void run();

				public:
					Texture fai;

		}; // end Pps

		// the stages as data members
		Ms ms; ///< Material rendering stage
		Is is; ///< Illumination rendering stage
		Pps pps; ///< Postprocessing rendering stage

	//===================================================================================================================================
	//                                                                                                                                  =
	//===================================================================================================================================
	public:
		// quality
		uint  width; ///< width of the rendering. Dont confuse with the window width
		uint  height; ///< height of the rendering. Dont confuse with the window width
		float renderingQuality; ///< The global rendering quality of the raster image. From 0.0(low) to 1.0(high)
		static int screenshotJpegQuality; ///< The quality of the JPEG screenshots. From 0 to 100
		// texture stuff
		static bool textureCompression; ///< Used in Texture::load to enable texture compression. Decreases video memory usage
		static int  maxTextureUnits; ///< Used in Texture::bind so we wont bind in a nonexistent texture unit. Readonly
		static bool mipmapping; ///< Used in Texture::load. Enables mipmapping increases video memory usage
		static int  maxAnisotropy; ///< Max texture anisotropy. Used in Texture::load
		// other
		static int   maxColorAtachments; ///< Max color attachments a FBO can accept
		uint  framesNum;
		float aspectRatio;
		static float quadVertCoords [][2];
		// matrices & viewing
		Camera* cam; ///< Current camera
		Mat4 modelViewMat; ///< This changes once for every mesh rendering
		Mat4 projectionMat; ///< This changes once every frame
		Mat4 modelViewProjectionMat; ///< This changes just like @ref modelViewMat
		Mat3 normalMat; ///< The rotation part of modelViewMat

		void setProjectionMatrix( const Camera& cam ) {}
		void setViewMatrix( const Camera& cam ) {}
		void setProjectionViewMatrices( const Camera& cam ) {}
		static void setViewport( uint x, uint y, uint w, uint h ) {}
		static void multMatrix( const Mat4& ) {}
		static void drawQuad( int vertCoordsUniLoc ) {};
		static void noShaders() {}

};

#endif
