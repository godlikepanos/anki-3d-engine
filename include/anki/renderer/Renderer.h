// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RENDERER_RENDERER_H
#define ANKI_RENDERER_RENDERER_H

#include "anki/renderer/Common.h"
#include "anki/renderer/Drawer.h"
#include "anki/Math.h"
#include "anki/Gr.h"
#include "anki/scene/Forward.h"
#include "anki/resource/Forward.h"
#include "anki/resource/ShaderResource.h"
#include "anki/core/Timestamp.h"

namespace anki {

// Forward
class ConfigSet;
class ResourceManager;

/// @addtogroup renderer
/// @{

/// Offscreen renderer. It is a class and not a namespace because we may need
/// external renderers for security cameras for example
class Renderer
{
public:
	const U TILE_SIZE = 64;

	Renderer();

	~Renderer();

	const Ms& getMs() const
	{
		return *m_ms;
	}

	Ms& getMs()
	{
		return *m_ms;
	}

	const Is& getIs() const
	{
		return *m_is;
	}

	Is& getIs()
	{
		return *m_is;
	}

	const Tiler& getTiler() const
	{
		return *m_tiler;
	}

	Tiler& getTiler()
	{
		return *m_tiler;
	}

	const Pps& getPps() const
	{
		return *m_pps;
	}

	Pps& getPps()
	{
		return *m_pps;
	}

	const Dbg& getDbg() const
	{
		return *m_dbg;
	}

	Dbg& getDbg()
	{
		return *m_dbg;
	}

	U32 getWidth() const
	{
		return m_width;
	}

	U32 getHeight() const
	{
		return m_height;
	}

	F32 getAspectRatio() const
	{
		return F32(m_width) / m_height;
	}

	U32 getFramesCount() const
	{
		return m_framesNum;
	}

	const SceneNode& getActiveCamera() const
	{
		return *m_frustumable;
	}

	SceneNode& getActiveCamera()
	{
		return *m_frustumable;
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

	UVec2 getTileSize() const
	{
		return UVec2(TILE_SIZE, TILE_SIZE);
	}

	const ShaderPtr& getDrawQuadVertexShader() const
	{
		return m_drawQuadVert->getGrShader();
	}

	/// Set the output of the renderer.
	void setOutputFramebuffer(FramebufferPtr outputFb, U32 width, U32 height)
	{
		m_outputFb = outputFb;
		m_outputFbSize = UVec2(width, height);
	}

	void getOutputFramebuffer(FramebufferPtr& outputFb, U32& width, U32& height)
	{
		if(m_outputFb.isCreated())
		{
			outputFb = m_outputFb;
			width = m_outputFbSize.x();
			height = m_outputFbSize.y();
		}
	}

	/// This function does all the rendering stages and produces a final result.
	ANKI_USE_RESULT Error render(
		SceneNode& frustumableNode,
		Array<CommandBufferPtr, RENDERER_COMMAND_BUFFERS_COUNT>& cmdBuff);

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
	void drawQuad(CommandBufferPtr& cmdBuff);

	void drawQuadConditional(
		OcclusionQueryPtr& q, CommandBufferPtr& cmdBuff);

	void drawQuadInstanced(CommandBufferPtr& cmdBuff, U32 primitiveCount);

	/// Get the LOD given the distance of an object from the camera
	F32 calculateLod(F32 distance) const
	{
		return distance / m_lodDistance;
	}

	/// Create a framebuffer attachment texture
	ANKI_USE_RESULT Error createRenderTarget(U32 w, U32 h,
		const PixelFormat& format, U32 samples, SamplingFilter filter,
		U mipsCount, TexturePtr& rt);

	/// Create a pipeline object that has as a vertex shader the m_drawQuadVert
	/// and the given fragment progam
	ANKI_USE_RESULT Error createDrawQuadPipeline(
		ShaderPtr frag, PipelinePtr& ppline);

	/// Init the renderer given an initialization class
	/// @param initializer The initializer class
	ANKI_USE_RESULT Error init(
		Threadpool* threadpool,
		ResourceManager* resources,
		GrManager* gl,
		HeapAllocator<U8> alloc,
		StackAllocator<U8> frameAlloc,
		const ConfigSet& config,
		const Timestamp* globalTimestamp);

	void prepareForVisibilityTests(Camera& cam);

	/// @privatesection
	/// @{
	GrManager& getGrManager()
	{
		return *m_gr;
	}

	HeapAllocator<U8>& getAllocator()
	{
		return m_alloc;
	}

	StackAllocator<U8>& getFrameAllocator()
	{
		return m_frameAlloc;
	}

	ResourceManager& getResourceManager()
	{
		return *m_resources;
	}

	Threadpool& getThreadpool()
	{
		return *m_threadpool;
	}

	Timestamp getGlobalTimestamp() const
	{
		return *m_globalTimestamp;
	}
	/// @}

private:
	Threadpool* m_threadpool;
	ResourceManager* m_resources;
	GrManager* m_gr;
	HeapAllocator<U8> m_alloc;
	StackAllocator<U8> m_frameAlloc;
	const Timestamp* m_globalTimestamp = nullptr;

	/// @name Rendering stages
	/// @{
	UniquePtr<Ms> m_ms; ///< Material rendering stage
	UniquePtr<Is> m_is; ///< Illumination rendering stage
	UniquePtr<Pps> m_pps; ///< Postprocessing rendering stage
	UniquePtr<Fs> m_fs; ///< Forward shading.
	UniquePtr<Lf> m_lf; ///< Forward shading lens flares.
	UniquePtr<Tiler> m_tiler;
	UniquePtr<Dbg> m_dbg; ///< Debug stage.
	/// @}

	U32 m_width;
	U32 m_height;

	F32 m_lodDistance; ///< Distance that used to calculate the LOD
	U8 m_samples; ///< Number of sample in multisampling
	Bool8 m_isOffscreen; ///< Is offscreen renderer?
	Bool8 m_tessellation;
	UVec2 m_tilesCount;
	U32 m_tilesCountXY;

	/// @name For drawing a quad into the active framebuffer
	/// @{
	BufferPtr m_quadPositionsBuff; ///< The VBO for quad positions
	ShaderResourcePointer m_drawQuadVert;
	/// @}

	/// @name Optimization vars
	/// Used in other stages
	/// @{

	/// A vector that contains useful numbers for calculating the view space
	/// position from the depth
	Vec4 m_projectionParams = Vec4(0.0);

	Timestamp m_projectionParamsUpdateTimestamp = 0;
	/// @}

	SceneNode* m_frustumable = nullptr; ///< Cache current frustumable node.
	RenderableDrawer m_sceneDrawer;

	U m_framesNum; ///< Frame number

	FramebufferPtr m_outputFb;
	UVec2 m_outputFbSize;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);
};
/// @}

} // end namespace anki

#endif
