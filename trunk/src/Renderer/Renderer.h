#ifndef RENDERER_H
#define RENDERER_H

#include "Common.h"
#include "Math.h"
#include "Fbo.h"
#include "Texture.h"
#include "ShaderProg.h"
#include "Vbo.h"
#include "RsrcPtr.h"
#include "Object.h"
#include "Ms.h"
#include "Is.h"
#include "Pps.h"
#include "Bs.h"
#include "Dbg.h"


class Camera;
class RendererInitializer;
class SceneNode;


/// Offscreen renderer
/// It is a class and not a namespace because we may need external renderers for security cameras for example
class Renderer: public Object
{
	//====================================================================================================================
	// Properties                                                                                                        =
	//====================================================================================================================
	PROPERTY_R(uint, width, getWidth) ///< Width of the rendering. Dont confuse with the window width
	PROPERTY_R(uint, height, getHeight) ///< Height of the rendering. Dont confuse with the window width
	PROPERTY_R(float, aspectRatio, getAspectRatio) ///< Just a precalculated value

	//====================================================================================================================
	// Public                                                                                                            =
	//====================================================================================================================
	public:
		/// @name Rendering stages
		/// @{
		Ms ms; ///< Material rendering stage
		Is is; ///< Illumination rendering stage
		Pps pps; ///< Postprocessing rendering stage
		Bs bs; ///< Blending stage
		/// @}

		static float quadVertCoords [][2];

		Renderer(Object* parent);

		~Renderer() throw() {}

		/// @name Setters & getters
		/// @{
		const Camera& getCamera() const {return *cam;}
		/// @}

		/// Init the renderer given an initialization class
		/// @param initializer The initializer class
		void init(const RendererInitializer& initializer);

		/// This function does all the rendering stages and produces a final FAI
		/// @param cam The camera from where the rendering will be done
		void render(Camera& cam);

		/// @name Setters & getters
		/// @{
		uint getFramesNum() const {return framesNum;}
		/// @}

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

		/// @todo write cmnts
		/// @param mtl
		/// @param sceneNode
		/// @param cam
		void setupMaterial(const class Material& mtl, const SceneNode& sceneNode, const Camera& cam);


		/// Draws a quad
		/// @param vertCoordsAttribLoc The attribute location of the vertex positions
		static void drawQuad(int vertCoordsAttribLoc);


	//====================================================================================================================
	// Protected                                                                                                         =
	//====================================================================================================================
	protected:
		uint framesNum; ///< Frame number
		const Camera* cam; ///< Current camera
		static int maxColorAtachments; ///< Max color attachments a FBO can accept
		Mat4 viewProjectionMat; ///< In case anyone needs it

		// to be removed
	public:
		static void color3(const Vec3& v) { glColor3fv(&((Vec3&)v)[0]); } ///< OpenGL wrapper
		static void color4(const Vec4& v) { glColor4fv(&((Vec4&)v)[0]); } ///< OpenGL wrapper
		static void setProjectionMatrix(const Camera& cam);
		static void setViewMatrix(const Camera& cam);
		static void noShaders() { ShaderProg::unbind(); } ///< unbind shaders @todo remove this. From now on there will be only shaders
		static void setProjectionViewMatrices(const Camera& cam) { setProjectionMatrix(cam); setViewMatrix(cam); }
		static void multMatrix(const Mat4& m4) { glMultMatrixf(&(m4.getTransposed())(0, 0)); } ///< OpenGL wrapper
		static void multMatrix(const Transform& trf) { glMultMatrixf(&(Mat4(trf).getTransposed())(0, 0)); } ///< OpenGL wrapper
		static void loadMatrix(const Mat4& m4) { glLoadMatrixf(&(m4.getTransposed())(0, 0)); } ///< OpenGL wrapper
		static void loadMatrix(const Transform& trf) { glLoadMatrixf(&(Mat4(trf).getTransposed())(0, 0)); } ///< OpenGL wrapper
};

#endif
