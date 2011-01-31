#ifndef RENDERER_H
#define RENDERER_H

#include "Math.h"
#include "Fbo.h"
#include "Texture.h"
#include "ShaderProg.h"
#include "Vbo.h"
#include "Vao.h"
#include "RsrcPtr.h"
#include "Object.h"
#include "Ms.h"
#include "Is.h"
#include "Pps.h"
#include "Bs.h"
#include "Dbg.h"
#include "GlException.h"
#include "SceneDrawer.h"


class Camera;
class RendererInitializer;
class ModelNode;


/// Offscreen renderer
/// It is a class and not a namespace because we may need external renderers for security cameras for example
class Renderer
{
	//====================================================================================================================
	// Public                                                                                                            =
	//====================================================================================================================
	public:
		/// The types of rendering a ModelNode
		enum ModelNodeRenderType
		{
			MNRT_MS, ///< In material stage
			MNRT_DP, ///< In a depth pass
			MNRT_BS  ///< In blending stage
		};

		Renderer();
		~Renderer() throw() {}

		/// @name Accessors
		/// @{
		GETTER_RW(Ms, ms, getMs)
		GETTER_RW(Is, is, getIs)
		GETTER_RW(Pps, pps, getPps)
		uint getWidth() const {return width;}
		uint getHeight() const {return height;}
		float getAspectRatio() const {return aspectRatio;}
		uint getFramesNum() const {return framesNum;}
		GETTER_R(Mat4, viewProjectionMat, getViewProjectionMat);
		const Camera& getCamera() const {return *cam;}
		GETTER_R(SceneDrawer, sceneDrawer, getSceneDrawer)
		/// @}

		/// Init the renderer given an initialization class
		/// @param initializer The initializer class
		void init(const RendererInitializer& initializer);

		/// This function does all the rendering stages and produces a final FAI
		/// @param cam The camera from where the rendering will be done
		void render(Camera& cam);

		/// My version of gluUnproject
		/// @param windowCoords Window screen coords
		/// @param modelViewMat The modelview matrix
		/// @param projectionMat The projection matrix
		/// @param view The view vector
		/// @return The unprojected coords
		static Vec3 unproject(const Vec3& windowCoords, const Mat4& modelViewMat, const Mat4& projectionMat,
		                      const int view[4]);

		/// It returns an orthographic projection matrix
		/// @param left left vertical clipping plane
		/// @param right right vertical clipping plane
		/// @param bottom bottom horizontal clipping plane
		/// @param top top horizontal clipping plane
		/// @param near nearer distance of depth clipping plane
		/// @param far farther distance of depth clipping plane
		/// @return A 4x4 projection matrix
		static Mat4 ortho(float left, float right, float bottom, float top, float near, float far);

		/// OpenGL wrapper
		static void setViewport(uint x, uint y, uint w, uint h) {glViewport(x, y, w, h);}

		/// Draws a quad. Actually it draws 2 triangles because OpenGL will no longer support quads
		/// @param vertCoordsAttribLoc The attribute location of the vertex positions
		void drawQuad();

	//====================================================================================================================
	// Protected                                                                                                         =
	//====================================================================================================================
	protected:
		/// @name Rendering stages
		/// @{
		Ms ms; ///< Material rendering stage
		Is is; ///< Illumination rendering stage
		Pps pps; ///< Postprocessing rendering stage
		Bs bs; ///< Blending stage
		/// @}

		uint width; ///< Width of the rendering. Dont confuse with the window width
		uint height; ///< Height of the rendering. Dont confuse with the window width
		float aspectRatio; ///< Just a precalculated value
		const Camera* cam; ///< Current camera
		static int maxColorAtachments; ///< Max color attachments an FBO can accept
		SceneDrawer sceneDrawer;

	//====================================================================================================================
	// Protected                                                                                                         =
	//====================================================================================================================
	private:
		uint framesNum; ///< Frame number
		Mat4 viewProjectionMat; ///< Precalculated in case anyone needs it

		/// @name For drawing a quad into the active framebuffer
		/// @{
		Vbo quadPositionsVbo; ///< The VBO for quad positions
		Vbo quadVertIndecesVbo; ///< The VBO for quad array buffer elements
		Vao quadVao; ///< This VAO is used everywhere except material stage
		/// @}
};


#endif
