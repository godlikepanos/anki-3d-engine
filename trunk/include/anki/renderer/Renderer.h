#ifndef ANKI_RENDERER_RENDERER_H
#define ANKI_RENDERER_RENDERER_H

#include "anki/Math.h"
#include "anki/resource/TextureResource.h"
#include "anki/resource/ShaderProgramResource.h"
#include "anki/resource/Resource.h"
#include "anki/gl/Gl.h"
#include "anki/util/HighRezTimer.h"
#include "anki/misc/ConfigSet.h"

#include "anki/renderer/Common.h"
#include "anki/renderer/Ms.h"
#include "anki/renderer/Is.h"
#include "anki/renderer/Pps.h"
#include "anki/renderer/Bs.h"
#include "anki/renderer/Dbg.h"
#include "anki/renderer/Tiler.h"
#include "anki/renderer/Drawer.h"

namespace anki {

/// A struct to initialize the renderer. It contains a few extra params for
/// the MainRenderer. Open Renderer.cpp to see all the options
class RendererInitializer: public ConfigSet
{
public:
	RendererInitializer();
};

/// Offscreen renderer. It is a class and not a namespace because we may need
/// external renderers for security cameras for example
class Renderer
{
public:
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

	const Tiler& getTiler() const
	{
		return tiler;
	}
	Tiler& getTiler()
	{
		return tiler;
	}

	const Pps& getPps() const
	{
		return pps;
	}
	Pps& getPps()
	{
		return pps;
	}

	const Dbg& getDbg() const
	{
		return dbg;
	}
	Dbg& getDbg()
	{
		return dbg;
	}

	U32 getWidth() const
	{
		return width;
	}

	U32 getHeight() const
	{
		return height;
	}

	U32 getWindowWidth() const
	{
		ANKI_ASSERT(!isOffscreen);
		return width / renderingQuality;
	}

	U32 getWindowHeight() const
	{
		ANKI_ASSERT(!isOffscreen);
		return height / renderingQuality;
	}

	F32 getAspectRatio() const
	{
		return F32(width) / height;
	}

	U32 getFramesCount() const
	{
		return framesNum;
	}

	const SceneGraph& getSceneGraph() const
	{
		return *scene;
	}
	SceneGraph& getSceneGraph()
	{
		return *scene;
	}

	const RenderableDrawer& getSceneDrawer() const
	{
		return sceneDrawer;
	}
	RenderableDrawer& getSceneDrawer()
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
	Timestamp getPlanesUpdateTimestamp() const
	{
		return planesUpdateTimestamp;
	}

	U getSamples() const
	{
		return samples;
	}

	Bool getUseMrt() const
	{
		return useMrt;
	}

	Bool getIsOffscreen() const
	{
		return isOffscreen;
	}

	F32 getRenderingQuality() const
	{
		return renderingQuality;
	}

	U32 getMaxTextureSize() const
	{
		return maxTextureSize;
	}
	/// @}

	/// Init the renderer given an initialization class
	/// @param initializer The initializer class
	void init(const RendererInitializer& initializer);

	/// This function does all the rendering stages and produces a final FAI
	void render(SceneGraph& scene);

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

	void drawQuadInstanced(U32 primitiveCount);

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
	static void calcLimitsOfNearPlane(const class PerspectiveCamera& cam,
		Vec2& limitsOfNearPlane);

	/// Get the LOD given the distance of an object from the camera
	U calculateLod(F32 distance) const
	{
		return distance / lodDistance;
	}

	/// On some GPUs its optimal to clean after binding to an FBO and there is
	/// no use for it's previous contents. For other GPUs the clear will be 
	/// skipped
	void clearAfterBindingFbo(const GLenum cap);

private:
	/// @name Rendering stages
	/// @{
	Ms ms; ///< Material rendering stage
	Is is; ///< Illumination rendering stage
	Pps pps; ///< Postprocessing rendering stage
	Bs bs; ///< Blending stage
	Dbg dbg; ///< Debug stage
	Tiler tiler;
	/// @}

	/// Width of the rendering. Don't confuse with the window width
	U32 width;
	/// Height of the rendering. Don't confuse with the window height
	U32 height;
	F32 lodDistance; ///< Distance that used to calculate the LOD
	U8 samples; ///< Number of sample in multisampling
	Bool8 useMrt; ///< Use MRT or pack things inside the G buffer
	Bool8 isOffscreen; ///< Is offscreen renderer?
	F32 renderingQuality; ///< Rendering quality. Relevant for offscreen 
	U32 maxTextureSize; ///< Texture size limit. Just kept here.
	U16 tilesXCount;
	U16 tilesYCount;

	/// @name For drawing a quad into the active framebuffer
	/// @{
	Vbo quadPositionsVbo; ///< The VBO for quad positions
	Vao quadVao; ///< This VAO is used everywhere except material stage
	/// @}

	/// @name Optimization vars
	/// Used in other stages
	/// @{
	Timestamp planesUpdateTimestamp = getGlobTimestamp();

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

	SceneGraph* scene; ///< Current scene
	RenderableDrawer sceneDrawer;

	U framesNum; ///< Frame number
};

} // end namespace anki

#endif
