// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RENDERER_RENDERER_H
#define ANKI_RENDERER_RENDERER_H

#include "anki/renderer/Common.h"
#include "anki/Math.h"
#include "anki/resource/ResourceManager.h"
#include "anki/resource/TextureResource.h"
#include "anki/resource/ShaderResource.h"
#include "anki/Gr.h"
#include "anki/util/HighRezTimer.h"
#include "anki/scene/Forward.h"

#include "anki/renderer/Ms.h"
#include "anki/renderer/Dp.h"
#include "anki/renderer/Is.h"
#include "anki/renderer/Pps.h"
#include "anki/renderer/Dbg.h"
#include "anki/renderer/Tiler.h"
#include "anki/renderer/Drawer.h"

namespace anki {

// Forward
class ConfigSet;

/// @addtogroup renderer
/// @{

/// Offscreen renderer. It is a class and not a namespace because we may need
/// external renderers for security cameras for example
class Renderer
{
public:
	/// Cut the job submition into multiple chains. We want to avoid feeding
	/// GL with a huge job chain
	static const U32 JOB_CHAINS_COUNT = 2;
		
	Renderer();

	~Renderer();

	const Ms& getMs() const
	{
		return m_ms;
	}
	Ms& getMs()
	{
		return m_ms;
	}

	const Dp& getDp() const
	{
		return m_dp;
	}
	Dp& getDp()
	{
		return m_dp;
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

	U32 getDefaultFramebufferWidth() const
	{
		ANKI_ASSERT(!m_isOffscreen);
		return m_defaultFbWidth;
	}

	U32 getDefaultFramebufferHeight() const
	{
		ANKI_ASSERT(!m_isOffscreen);
		return m_defaultFbHeight;
	}

	F32 getRenderingQuality() const
	{
		return m_renderingQuality;
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

	Timestamp getProjectionParametersUpdateTimestamp() const
	{
		return m_projectionParamsUpdateTimestamp;
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

	Bool getTessellationEnabled() const
	{
		return m_tessellation;
	}

	const UVec2& getTilesCount() const
	{
		return m_tilesCount;
	}

	U getTilesCountXY() const
	{
		return m_tilesCountXY;
	}

	const ShaderHandle& getDrawQuadVertexShader() const
	{
		return m_drawQuadVert->getGrShader();
	}

	FramebufferHandle& getDefaultFramebuffer()
	{
		return m_defaultFb;
	}

	/// This function does all the rendering stages and produces a final FAI
	ANKI_USE_RESULT Error render(SceneGraph& scene, 
		Array<CommandBufferHandle, JOB_CHAINS_COUNT>& cmdBuff);

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
	void drawQuad(CommandBufferHandle& cmdBuff);

	void drawQuadConditional(
		OcclusionQueryHandle& q, CommandBufferHandle& cmdBuff);

	void drawQuadInstanced(CommandBufferHandle& cmdBuff, U32 primitiveCount);

	/// Get the LOD given the distance of an object from the camera
	F32 calculateLod(F32 distance) const
	{
		return distance / m_lodDistance;
	}

	/// Create a framebuffer attachment texture
	ANKI_USE_RESULT Error createRenderTarget(U32 w, U32 h, 
		const PixelFormat& format, U32 samples, SamplingFilter filter,
		U mipsCount, TextureHandle& rt);

	/// Create a pipeline object that has as a vertex shader the m_drawQuadVert
	/// and the given fragment progam
	ANKI_USE_RESULT Error createDrawQuadPipeline(
		ShaderHandle frag, PipelineHandle& ppline);

	/// Init the renderer given an initialization class
	/// @param initializer The initializer class
	ANKI_USE_RESULT Error init(
		Threadpool* threadpool, 
		ResourceManager* resources,
		GrManager* gl,
		HeapAllocator<U8>& alloc,
		const ConfigSet& config,
		const Timestamp* globalTimestamp);

	/// @privatesection
	/// @{
	GrManager& _getGrManager()
	{
		return *m_gl;
	}

	HeapAllocator<U8>& _getAllocator()
	{
		return m_alloc;
	}

	ResourceManager& _getResourceManager()
	{
		return *m_resources;
	}

	Threadpool& _getThreadpool() 
	{
		return *m_threadpool;
	}

	const String& _getShadersPrependedSource() const
	{
		return m_shadersPrependedSource;
	}

	Timestamp getGlobalTimestamp() const
	{
		return *m_globalTimestamp;
	}
	/// @}

private:
	Threadpool* m_threadpool;
	ResourceManager* m_resources;
	GrManager* m_gl;
	HeapAllocator<U8> m_alloc;
	const Timestamp* m_globalTimestamp = nullptr;

	/// @name Rendering stages
	/// @{
	Ms m_ms; ///< Material rendering stage
	Dp m_dp; ///< Depth processing stage.
	Is m_is; ///< Illumination rendering stage
	Pps m_pps; ///< Postprocessing rendering stage
	Fs* m_fs = nullptr; ///< Forward shading.
	Dbg m_dbg; ///< Debug stage
	Tiler m_tiler;
	/// @}

	U32 m_width;
	U32 m_height;
	U32 m_defaultFbWidth;
	U32 m_defaultFbHeight;
	F32 m_renderingQuality;

	F32 m_lodDistance; ///< Distance that used to calculate the LOD
	U8 m_samples; ///< Number of sample in multisampling
	Bool8 m_isOffscreen; ///< Is offscreen renderer?
	Bool8 m_tessellation;
	UVec2 m_tilesCount;
	U32 m_tilesCountXY;

	/// @name For drawing a quad into the active framebuffer
	/// @{
	BufferHandle m_quadPositionsBuff; ///< The VBO for quad positions
	ShaderResourcePointer m_drawQuadVert;
	/// @}

	/// @name Optimization vars
	/// Used in other stages
	/// @{

	/// A vector that contains useful numbers for calculating the view space
	/// position from the depth
	Vec4 m_projectionParams;

	Timestamp m_projectionParamsUpdateTimestamp = 0;
	/// @}

	SceneGraph* m_scene; ///< Current scene
	RenderableDrawer m_sceneDrawer;

	U m_framesNum; ///< Frame number

	FramebufferHandle m_defaultFb;

	String m_shadersPrependedSource; ///< String to append in user shaders

	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);
};

/// @}

} // end namespace anki

#endif
