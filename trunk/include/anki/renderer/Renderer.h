#ifndef ANKI_RENDERER_RENDERER_H
#define ANKI_RENDERER_RENDERER_H

#include "anki/Math.h"
#include "anki/resource/TextureResource.h"
#include "anki/resource/ProgramResource.h"
#include "anki/resource/Resource.h"
#include "anki/Gl.h"
#include "anki/util/HighRezTimer.h"
#include "anki/misc/ConfigSet.h"
#include "anki/scene/Forward.h"

#include "anki/renderer/Ms.h"
#include "anki/renderer/Is.h"
#include "anki/renderer/Pps.h"
#include "anki/renderer/Bs.h"
#include "anki/renderer/Dbg.h"
#include "anki/renderer/Tiler.h"
#include "anki/renderer/Drawer.h"

namespace anki {

/// @addtogroup renderer
/// @{

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
		return m_ms;
	}
	Ms& getMs()
	{
		return m_ms;
	}

	const Is& getIs() const
	{
		return m_is;
	}
	Is& getIs()
	{
		return m_is;
	}

	const Tiler& getTiler() const
	{
		return m_tiler;
	}
	Tiler& getTiler()
	{
		return m_tiler;
	}

	const Pps& getPps() const
	{
		return m_pps;
	}
	Pps& getPps()
	{
		return m_pps;
	}

	const Dbg& getDbg() const
	{
		return m_dbg;
	}
	Dbg& getDbg()
	{
		return m_dbg;
	}

	U32 getWidth() const
	{
		return m_width;
	}

	U32 getHeight() const
	{
		return m_height;
	}

	U32 getWindowWidth() const
	{
		ANKI_ASSERT(!m_isOffscreen);
		return m_width / m_renderingQuality;
	}

	U32 getWindowHeight() const
	{
		ANKI_ASSERT(!m_isOffscreen);
		return m_height / m_renderingQuality;
	}

	F32 getAspectRatio() const
	{
		return F32(m_width) / m_height;
	}

	U32 getFramesCount() const
	{
		return m_framesNum;
	}

	const SceneGraph& getSceneGraph() const
	{
		return *m_scene;
	}
	SceneGraph& getSceneGraph()
	{
		return *m_scene;
	}

	const RenderableDrawer& getSceneDrawer() const
	{
		return m_sceneDrawer;
	}
	RenderableDrawer& getSceneDrawer()
	{
		return m_sceneDrawer;
	}

	const Vec2& getPlanes() const
	{
		return m_planes;
	}
	const Vec2& getLimitsOfNearPlane() const
	{
		return m_limitsOfNearPlane;
	}
	const Vec2& getLimitsOfNearPlane2() const
	{
		return m_limitsOfNearPlane2;
	}
	Timestamp getPlanesUpdateTimestamp() const
	{
		return m_planesUpdateTimestamp;
	}
	const Vec4& getProjectionParameters() const
	{
		return m_projectionParams;
	}

	U getSamples() const
	{
		return m_samples;
	}

	Bool getIsOffscreen() const
	{
		return m_isOffscreen;
	}

	Bool usesTessellation() const
	{
		return m_tessellation;
	}

	F32 getRenderingQuality() const
	{
		return m_renderingQuality;
	}

	U32 getMaxTextureSize() const
	{
		return m_maxTextureSize;
	}

	const UVec2& getTilesCount() const
	{
		return m_tilesCount;
	}

	/// Get string to pass to the material shaders
	const std::string& getShaderPostProcessorString() const
	{
		return m_shaderPostProcessorString;
	}

	const GlProgramHandle& getDrawQuadVertexProgram() const
	{
		return m_drawQuadVert->getGlProgram();
	}

	GlFramebufferHandle& getDefaultFramebuffer()
	{
		return m_defaultFb;
	}
	/// @}

	/// Init the renderer given an initialization class
	/// @param initializer The initializer class
	void init(const RendererInitializer& initializer);

	/// This function does all the rendering stages and produces a final FAI
	void render(SceneGraph& scene, GlJobChainHandle& jobs);

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
	void drawQuad(GlJobChainHandle& jobs);

	void drawQuadInstanced(GlJobChainHandle& jobs, U32 primitiveCount);

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
	F32 calculateLod(F32 distance) const
	{
		return distance / m_lodDistance;
	}

	/// Create a framebuffer attachment texture
	void createRenderTarget(U32 w, U32 h, GLenum internalFormat, 
		GLenum format, GLenum type, U32 samples, GlTextureHandle& rt);

	/// Create a pipeline object that has as a vertex shader the m_drawQuadVert
	/// and the given fragment progam
	GlProgramPipelineHandle createDrawQuadProgramPipeline(
		GlProgramHandle frag);

private:
	/// @name Rendering stages
	/// @{
	Ms m_ms; ///< Material rendering stage
	Is m_is; ///< Illumination rendering stage
	Pps m_pps; ///< Postprocessing rendering stage
	Bs m_bs; ///< Blending stage
	Dbg m_dbg; ///< Debug stage
	Tiler m_tiler;
	/// @}

	/// Width of the rendering. Don't confuse with the window width
	U32 m_width;
	/// Height of the rendering. Don't confuse with the window height
	U32 m_height;
	F32 m_lodDistance; ///< Distance that used to calculate the LOD
	U8 m_samples; ///< Number of sample in multisampling
	Bool8 m_isOffscreen; ///< Is offscreen renderer?
	Bool8 m_tessellation;
	F32 m_renderingQuality; ///< Rendering quality. Relevant for offscreen 
	U32 m_maxTextureSize; ///< Texture size limit. Just kept here.
	UVec2 m_tilesCount;

	/// @name For drawing a quad into the active framebuffer
	/// @{
	GlBufferHandle m_quadPositionsBuff; ///< The VBO for quad positions

	ProgramResourcePointer m_drawQuadVert;
	/// @}

	/// @name Optimization vars
	/// Used in other stages
	/// @{
	Timestamp m_planesUpdateTimestamp = getGlobTimestamp();

	/// Used to to calculate the frag pos in view space inside a few shader
	/// programs
	Vec2 m_planes;
	/// Used to to calculate the frag pos in view space inside a few shader
	/// programs
	Vec2 m_limitsOfNearPlane;
	/// Used to to calculate the frag pos in view space inside a few shader
	/// programs
	Vec2 m_limitsOfNearPlane2;

	/// A vector that contains useful numbers for calculating the view space
	/// position from the depth
	Vec4 m_projectionParams;
	/// @}

	SceneGraph* m_scene; ///< Current scene
	RenderableDrawer m_sceneDrawer;

	U m_framesNum; ///< Frame number

	/// String to pass to the material shaders
	std::string m_shaderPostProcessorString;

	GlFramebufferHandle m_defaultFb;

	void computeProjectionParams(const Mat4& projMat);
};

/// @}

} // end namespace anki

#endif
