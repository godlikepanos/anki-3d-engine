// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/Common.h>
#include <anki/renderer/Drawer.h>
#include <anki/renderer/Clusterer.h>
#include <anki/Math.h>
#include <anki/Gr.h>
#include <anki/scene/Forward.h>
#include <anki/resource/Forward.h>
#include <anki/resource/ShaderResource.h>
#include <anki/core/Timestamp.h>

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

	/// Init the renderer.
	ANKI_USE_RESULT Error init(
		ThreadPool* threadpool,
		ResourceManager* resources,
		GrManager* gr,
		HeapAllocator<U8> alloc,
		StackAllocator<U8> frameAlloc,
		const ConfigSet& config,
		const Timestamp* globalTimestamp);

	/// Set the output of the renderer before calling #render.
	void setOutputFramebuffer(FramebufferPtr outputFb, U32 width, U32 height)
	{
		m_outputFb = outputFb;
		m_outputFbSize = UVec2(width, height);
	}

	/// This function does all the rendering stages and produces a final result.
	ANKI_USE_RESULT Error render(
		SceneNode& frustumableNode, U frustumIdx,
		Array<CommandBufferPtr, RENDERER_COMMAND_BUFFERS_COUNT>& cmdBuff);

anki_internal:
	static const U TILE_SIZE = 64;

	void getOutputFramebuffer(FramebufferPtr& outputFb, U32& width, U32& height)
	{
		if(m_outputFb.isCreated())
		{
			outputFb = m_outputFb;
			width = m_outputFbSize.x();
			height = m_outputFbSize.y();
		}
	}

	U32 getFramesCount() const
	{
		return m_framesNum;
	}

	const FrustumComponent& getActiveFrustumComponent() const
	{
		ANKI_ASSERT(m_frc);
		return *m_frc;
	}

	FrustumComponent& getActiveFrustumComponent()
	{
		ANKI_ASSERT(m_frc);
		return *m_frc;
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

	U getTileCount() const
	{
		return m_tileCount;
	}

	const UVec2& getTileCountXY() const
	{
		return m_tileCountXY;
	}

	U getClusterCount() const
	{
		return m_clusterer.getClusterCount();
	}

	const Clusterer& getClusterer() const
	{
		return m_clusterer;
	}

	const ShaderPtr& getDrawQuadVertexShader() const
	{
		return m_drawQuadVert->getGrShader();
	}

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
	void drawQuad(CommandBufferPtr& cmdb)
	{
		drawQuadInstanced(cmdb, 1);
	}

	void drawQuadConditional(
		OcclusionQueryPtr& q, CommandBufferPtr& cmdb)
	{
		cmdb->drawArraysConditional(q, 3, 1);
	}

	void drawQuadInstanced(CommandBufferPtr& cmdb, U32 primitiveCount)
	{
		cmdb->drawArrays(3, primitiveCount);
	}

	/// Get the LOD given the distance of an object from the camera
	F32 calculateLod(F32 distance) const
	{
		return distance / m_lodDistance;
	}

	/// Create a pipeline object that has as a vertex shader the m_drawQuadVert
	/// and the given fragment progam
	void createDrawQuadPipeline(
		ShaderPtr frag,
		const ColorStateInfo& colorState,
		PipelinePtr& ppline);

	/// Create a framebuffer attachment texture
	void createRenderTarget(U32 w, U32 h,
		const PixelFormat& format, U32 samples, SamplingFilter filter,
		U mipsCount, TexturePtr& rt);

	Bool doGpuVisibilityTest(const CollisionShape& cs, const Aabb& aabb) const;

	void prepareForVisibilityTests(const SceneNode& cam);

	GrManager& getGrManager()
	{
		return *m_gr;
	}

	HeapAllocator<U8> getAllocator() const
	{
		return m_alloc;
	}

	StackAllocator<U8> getFrameAllocator() const
	{
		return m_frameAlloc;
	}

	ResourceManager& getResourceManager()
	{
		return *m_resources;
	}

	ThreadPool& getThreadPool()
	{
		return *m_threadpool;
	}

	Timestamp getGlobalTimestamp() const
	{
		return *m_globalTimestamp;
	}

	const Timestamp* getGlobalTimestampPtr()
	{
		return m_globalTimestamp;
	}

private:
	ThreadPool* m_threadpool;
	ResourceManager* m_resources;
	GrManager* m_gr;
	HeapAllocator<U8> m_alloc;
	StackAllocator<U8> m_frameAlloc;
	const Timestamp* m_globalTimestamp = nullptr;

	Clusterer m_clusterer;

	/// @name Rendering stages
	/// @{
	UniquePtr<Ms> m_ms; ///< Material rendering stage
	UniquePtr<Is> m_is; ///< Illumination rendering stage
	UniquePtr<Refl> m_refl; ///< Reflections.
	UniquePtr<Tiler> m_tiler;
	UniquePtr<Pps> m_pps; ///< Postprocessing rendering stage
	UniquePtr<Fs> m_fs; ///< Forward shading.
	UniquePtr<Lf> m_lf; ///< Forward shading lens flares.
	UniquePtr<Dbg> m_dbg; ///< Debug stage.
	/// @}

	U32 m_width;
	U32 m_height;

	F32 m_lodDistance; ///< Distance that used to calculate the LOD
	U8 m_samples; ///< Number of sample in multisampling
	Bool8 m_isOffscreen; ///< Is offscreen renderer?
	Bool8 m_tessellation;
	U32 m_tileCount;
	UVec2 m_tileCountXY;

	ShaderResourcePtr m_drawQuadVert;

	/// @name Optimization vars
	/// Used in other stages
	/// @{

	/// A vector that contains useful numbers for calculating the view space
	/// position from the depth
	Vec4 m_projectionParams = Vec4(0.0);

	Timestamp m_projectionParamsUpdateTimestamp = 0;
	/// @}

	FrustumComponent* m_frc = nullptr; ///< Cache current frustum component.
	RenderableDrawer m_sceneDrawer;

	U m_framesNum; ///< Frame number

	FramebufferPtr m_outputFb;
	UVec2 m_outputFbSize;

	TexturePtr m_reflectionsCubemapArr;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);
};
/// @}

} // end namespace anki

