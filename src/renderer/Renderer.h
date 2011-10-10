#ifndef RENDERER_H
#define RENDERER_H

#include "math/Math.h"
#include "gl/Fbo.h"
#include "resource/Texture.h"
#include "resource/ShaderProgram.h"
#include "gl/Vbo.h"
#include "gl/Vao.h"
#include "resource/RsrcPtr.h"
#include "Ms.h"
#include "Is.h"
#include "Pps.h"
#include "Bs.h"
#include "Dbg.h"
#include "gl/GlException.h"
#include "SceneDrawer.h"
#include "gl/GlStateMachine.h"
#include "gl/TimeQuery.h"
#include <boost/scoped_ptr.hpp>


class Camera;
struct RendererInitializer;
class ModelNode;


/// Renderer namespace

/// Offscreen renderer
/// It is a class and not a namespace because we may need external renderers
/// for security cameras for example
class Renderer
{
	public:
		/// The types of rendering a ModelNode
		enum ModelNodeRenderType
		{
			MNRT_MS, ///< In material stage
			MNRT_DP, ///< In a depth pass
			MNRT_BS  ///< In blending stage
		};

		Renderer();
		~Renderer();

		/// @name Accessors
		/// @{
		const Ms& getMs() const
		{
			return ms;
		}
		Ms& getMs()
		{
			return ms;
		}

		const Is& getIs() const
		{
			return is;
		}
		Is& getIs()
		{
			return is;
		}

		const Pps& getPps() const
		{
			return pps;
		}
		Pps& getPps()
		{
			return pps;
		}

		uint getWidth() const
		{
			return width;
		}

		uint getHeight() const
		{
			return height;
		}

		float getAspectRatio() const
		{
			return float(width) / height;
		}

		uint getFramesNum() const
		{
			return framesNum;
		}

		const Mat4& getViewProjectionMat() const
		{
			return viewProjectionMat;
		}

		const Camera& getCamera() const
		{
			return *cam;
		}

		const SceneDrawer& getSceneDrawer() const
		{
			return sceneDrawer;
		}
		SceneDrawer& getSceneDrawer()
		{
			return sceneDrawer;
		}

		const Vec2& getPlanes() const
		{
			return planes;
		}

		const Vec2& getLimitsOfNearPlane() const
		{
			return limitsOfNearPlane;
		}

		const Vec2& getLimitsOfNearPlane2() const
		{
			return limitsOfNearPlane2;
		}

		double getMsTime() const
		{
			return msTime;
		}

		double getIsTime() const
		{
			return isTime;
		}

		double getPpsTime() const
		{
			return ppsTime;
		}

		double getBsTime() const
		{
			return bsTime;
		}

		bool getStagesProfilingEnabled() const
		{
			return enableStagesProfilingFlag;
		}
		bool& getStagesProfilingEnabled()
		{
			return enableStagesProfilingFlag;
		}
		void setStagesProfilingEnabled(const bool x)
		{
			enableStagesProfilingFlag = x;
		}
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
		static Vec3 unproject(const Vec3& windowCoords,
			const Mat4& modelViewMat, const Mat4& projectionMat,
			const int view[4]);

		/// Draws a quad. Actually it draws 2 triangles because OpenGL will no
		/// longer support quads
		void drawQuad();

		/// Create FAI texture
		static void createFai(uint width, uint height, int internalFormat,
			int format, int type, Texture& fai);

		/// Calculate the planes needed for the calculation of the fragment
		/// position z in view space. Having the fragment's depth, the camera's
		/// zNear and zFar the z of the fragment is being calculated inside the
		/// fragment shader from:
		/// @code z = (- zFar * zNear) / (zFar - depth * (zFar - zNear))
		/// @endcode
		/// The above can be optimized and this method actually pre-calculates a
		/// few things in order to lift a few calculations from the fragment
		/// shader. So the z is:
		/// @code z =  -planes.y / (planes.x + depth) @endcode
		/// @param[in] cameraRange The zNear, zFar
		/// @param[out] planes The planes
		static void calcPlanes(const Vec2& cameraRange, Vec2& planes);

		/// Calculates two values needed for the calculation of the fragment
		/// position in view space.
		static void calcLimitsOfNearPlane(const PerspectiveCamera& cam,
			Vec2& limitsOfNearPlane);

	protected:
		/// @name Rendering stages
		/// @{
		Ms ms; ///< Material rendering stage
		Is is; ///< Illumination rendering stage
		Pps pps; ///< Postprocessing rendering stage
		Bs bs; ///< Blending stage
		/// @}

		/// @name Profiling stuff
		/// @{
		double msTime, isTime, ppsTime, bsTime;
		boost::scoped_ptr<TimeQuery> msTq, isTq, ppsTq, bsTq;
		bool enableStagesProfilingFlag;
		/// @}

		/// Width of the rendering. Don't confuse with the window width
		uint width;
		/// Height of the rendering. Don't confuse with the window width
		uint height;
		const Camera* cam; ///< Current camera
		/// Max color attachments an FBO can accept
		static int maxColorAtachments;
		SceneDrawer sceneDrawer;

		/// @name Optimization vars
		/// Used in other stages
		/// @{

		/// Used to to calculate the frag pos in view space inside a few shader
		/// programs
		Vec2 planes;
		/// Used to to calculate the frag pos in view space inside a few shader
		/// programs
		Vec2 limitsOfNearPlane;
		/// Used to to calculate the frag pos in view space inside a few shader
		/// programs
		Vec2 limitsOfNearPlane2;
		/// @}

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
