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


class Camera;
class RendererInitializer;
class ModelNode;
class ModelNode;


/// Offscreen renderer
/// It is a class and not a namespace because we may need external renderers for security cameras for example
class Renderer
{
	//====================================================================================================================
	// Properties                                                                                                        =
	//====================================================================================================================
	PROPERTY_R(uint, width, getWidth) ///< Width of the rendering. Dont confuse with the window width
	PROPERTY_R(uint, height, getHeight) ///< Height of the rendering. Dont confuse with the window width
	PROPERTY_R(float, aspectRatio, getAspectRatio) ///< Just a precalculated value
	PROPERTY_R(Mat4, viewProjectionMat, getViewProjectionMat) ///< Precalculated in case anyone needs it

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

		/// @name Accessors
		/// @{
		uint getFramesNum() const {return framesNum;}
		Ms& getMs() {return ms;}
		Is& getIs() {return is;}
		Pps& getPps() {return pps;}
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

		/// This function:
		/// - binds the shader program
		/// - loads the uniforms
		/// - sets the GL state
		/// @param mtl The material containing the shader program and the locations
		/// @param modelNode Needed for some matrices (model) and the bone stuff (rotations & translations)
		/// @param cam Needed for some matrices (view & projection)
		void setupShaderProg(const class Material& mtl, const ModelNode& modelNode, const Camera& cam) const;

		/// Render ModelNode. The method sets up the shader and renders the geometry
		void renderModelNode(const ModelNode& modelNode, const Camera& cam, ModelNodeRenderType type) const;

		/// Render all ModelNodes
		void renderAllModelNodes(const Camera& cam, ModelNodeRenderType type) const;

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

		uint framesNum; ///< Frame number
		const Camera* cam; ///< Current camera
		static int maxColorAtachments; ///< Max color attachments an FBO can accept

	//====================================================================================================================
	// Protected                                                                                                         =
	//====================================================================================================================
	private:
		/// @name For drawing a quad into the active framebuffer
		/// @{
		Vbo quadPositionsVbo; ///< The VBO for quad positions
		Vbo quadVertIndecesVbo; ///< The VBO for quad array buffer elements
		Vao quadVao; ///< This VAO is used everywhere except material stage
		/// @}
};


#endif
