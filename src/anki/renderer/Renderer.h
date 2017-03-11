// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/Common.h>
#include <anki/renderer/Drawer.h>
#include <anki/Math.h>
#include <anki/Gr.h>
#include <anki/scene/Forward.h>
#include <anki/resource/Forward.h>
#include <anki/resource/ShaderResource.h>
#include <anki/core/Timestamp.h>
#include <anki/core/StagingGpuMemoryManager.h>
#include <anki/util/ThreadPool.h>
#include <anki/collision/Forward.h>

namespace anki
{

// Forward
class ConfigSet;
class ResourceManager;
class StagingGpuMemoryManager;
class VisibilityTestResults;

/// @addtogroup renderer
/// @{

/// Rendering context.
class RenderingContext
{
public:
	const VisibilityTestResults* m_visResults ANKI_DBG_NULLIFY;
	Mat4 m_viewMat;
	Mat4 m_projMat;
	Mat4 m_viewProjMat;
	Mat4 m_camTrfMat;
	F32 m_near;
	F32 m_far;
	Vec4 m_unprojParams;

	Mat4 m_prevViewProjMat;
	Mat4 m_prevCamTransform;

	CommandBufferPtr m_commandBuffer; ///< Primary command buffer.

	StackAllocator<U8> m_tempAllocator;

	/// @name MS
	/// @{
	class Ms
	{
	public:
		Array<CommandBufferPtr, ThreadPool::MAX_THREADS> m_commandBuffers;
		U32 m_lastThreadWithWork = 0;
	} m_ms;
	/// @}

	/// @name LF
	/// @{
	class Lf
	{
	public:
		DynamicArrayAuto<OcclusionQueryPtr> m_queriesToTest;

		Lf(const StackAllocator<U8>& alloc)
			: m_queriesToTest(alloc)
		{
		}
	} m_lf;
	/// @}

	/// @name IS
	/// @{
	class Is
	{
	public:
		StagingGpuMemoryToken m_commonToken;
		StagingGpuMemoryToken m_pointLightsToken;
		StagingGpuMemoryToken m_spotLightsToken;
		StagingGpuMemoryToken m_probesToken;
		StagingGpuMemoryToken m_decalsToken;
		StagingGpuMemoryToken m_clustersToken;
		StagingGpuMemoryToken m_lightIndicesToken;

		TexturePtr m_diffDecalTex;
		TexturePtr m_normRoughnessDecalTex;
	} m_is;
	/// @}

	/// @name Shadow mapping
	/// @{
	class Sm
	{
	public:
		DynamicArrayAuto<FramebufferPtr> m_spotFramebuffers;
		DynamicArrayAuto<Array<FramebufferPtr, 6>> m_omniFramebuffers;

		DynamicArrayAuto<U> m_spotCacheIndices;
		DynamicArrayAuto<U> m_omniCacheIndices;

		/// [casterIdx][threadIdx]
		DynamicArrayAuto<CommandBufferPtr> m_spotCommandBuffers;
		/// [casterIdx][threadIdx][faceIdx]
		DynamicArrayAuto<CommandBufferPtr> m_omniCommandBuffers;

		DynamicArrayAuto<SceneNode*> m_spots;
		DynamicArrayAuto<SceneNode*> m_omnis;

		Sm(const StackAllocator<U8>& alloc)
			: m_spotFramebuffers(alloc)
			, m_omniFramebuffers(alloc)
			, m_spotCacheIndices(alloc)
			, m_omniCacheIndices(alloc)
			, m_spotCommandBuffers(alloc)
			, m_omniCommandBuffers(alloc)
			, m_spots(alloc)
			, m_omnis(alloc)
		{
		}
	} m_sm;
	/// @}

	/// @name FS
	/// @{
	class Fs
	{
	public:
		Array<CommandBufferPtr, ThreadPool::MAX_THREADS> m_commandBuffers;
		U32 m_lastThreadWithWork = 0;
	} m_fs;
	/// @}

	FramebufferPtr m_outFb;
	U32 m_outFbWidth = 0;
	U32 m_outFbHeight = 0;

	RenderingContext(const StackAllocator<U8>& alloc)
		: m_tempAllocator(alloc)
		, m_lf(alloc)
		, m_sm(alloc)
	{
	}
};

/// Offscreen renderer. It is a class and not a namespace because we may need external renderers for security cameras
/// for example
class Renderer
{
public:
	Renderer();

	~Renderer();

	Ir& getIr()
	{
		return *m_ir;
	}

	Sm& getSm()
	{
		return *m_sm;
	}

	Ms& getMs()
	{
		return *m_ms;
	}

	Is& getIs()
	{
		return *m_is;
	}

	DepthDownscale& getDepthDownscale()
	{
		return *m_depth;
	}

	Fs& getFs()
	{
		return *m_fs;
	}

	Volumetric& getVolumetric()
	{
		return *m_vol;
	}

	Tm& getTm()
	{
		return *m_tm;
	}

	Ssao& getSsao()
	{
		return *m_ssao;
	}

	Bloom& getBloom()
	{
		return *m_bloom;
	}

	Pps& getPps()
	{
		return *m_pps;
	}

	Dbg& getDbg()
	{
		return *m_dbg;
	}

	Smaa& getSmaa()
	{
		return *m_smaa;
	}

	DownscaleBlur& getDownscaleBlur()
	{
		return *m_downscale;
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
	ANKI_USE_RESULT Error init(ThreadPool* threadpool,
		ResourceManager* resources,
		GrManager* gr,
		StagingGpuMemoryManager* stagingMem,
		HeapAllocator<U8> alloc,
		StackAllocator<U8> frameAlloc,
		const ConfigSet& config,
		Timestamp* globTimestamp,
		Bool willDrawToDefaultFbo);

	/// This function does all the rendering stages and produces a final result.
	ANKI_USE_RESULT Error render(RenderingContext& ctx);

anki_internal:
	U64 getFrameCount() const
	{
		return m_frameCount;
	}

	const RenderableDrawer& getSceneDrawer() const
	{
		return m_sceneDrawer;
	}

	RenderableDrawer& getSceneDrawer()
	{
		return m_sceneDrawer;
	}

	Bool getTessellationEnabled() const
	{
		return m_tessellation;
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
	static Vec3 unproject(
		const Vec3& windowCoords, const Mat4& modelViewMat, const Mat4& projectionMat, const int view[4]);

	/// Draws a quad. Actually it draws 2 triangles because OpenGL will no longer support quads
	void drawQuad(CommandBufferPtr& cmdb)
	{
		drawQuadInstanced(cmdb, 1);
	}

	void drawQuadInstanced(CommandBufferPtr& cmdb, U32 primitiveCount)
	{
		cmdb->drawArrays(PrimitiveTopology::TRIANGLES, 3, primitiveCount);
	}

	/// Get the LOD given the distance of an object from the camera
	F32 calculateLod(F32 distance) const
	{
		return distance / m_lodDistance;
	}

	/// Create a shader program that has as a vertex shader the m_drawQuadVert and the given fragment progam
	void createDrawQuadShaderProgram(ShaderPtr frag, ShaderProgramPtr& prog);

	/// Create the init info for a 2D texture that will be used as a render target.
	ANKI_USE_RESULT TextureInitInfo create2DRenderTargetInitInfo(U32 w,
		U32 h,
		const PixelFormat& format,
		TextureUsageBit usage,
		SamplingFilter filter,
		U mipsCount = 1,
		CString name = {});

	ANKI_USE_RESULT TexturePtr createAndClearRenderTarget(const TextureInitInfo& inf);

	ANKI_USE_RESULT Error createShader(CString fname, ShaderResourcePtr& shader, CString extra);
	ANKI_USE_RESULT Error createShaderf(CString fname, ShaderResourcePtr& shader, CString fmt, ...);

	GrManager& getGrManager()
	{
		return *m_gr;
	}

	StagingGpuMemoryManager& getStagingGpuMemoryManager()
	{
		return *m_stagingMem;
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
		return *m_globTimestamp;
	}

	Timestamp* getGlobalTimestampPtr()
	{
		return m_globTimestamp;
	}

	/// Returns true if there were resources loaded or loading async tasks that got completed.
	Bool resourcesLoaded() const
	{
		return m_resourcesDirty;
	}

	Bool getDrawToDefaultFramebuffer() const
	{
		return m_willDrawToDefaultFbo;
	}

	TexturePtr getDummyTexture() const
	{
		return m_dummyTex;
	}

	BufferPtr getDummyBuffer() const
	{
		return m_dummyBuff;
	}

	static constexpr PtrSize getDummyBufferSize()
	{
		return 1024;
	}

private:
	ThreadPool* m_threadpool = nullptr;
	ResourceManager* m_resources = nullptr;
	GrManager* m_gr = nullptr;
	StagingGpuMemoryManager* m_stagingMem = nullptr;
	Timestamp* m_globTimestamp;
	HeapAllocator<U8> m_alloc;
	StackAllocator<U8> m_frameAlloc;

	/// @name Rendering stages
	/// @{
	UniquePtr<Ir> m_ir;
	UniquePtr<Sm> m_sm; ///< Shadow mapping.
	UniquePtr<Ms> m_ms; ///< Material rendering stage
	UniquePtr<Is> m_is; ///< Illumination rendering stage
	UniquePtr<DepthDownscale> m_depth;
	UniquePtr<Fs> m_fs; ///< Forward shading.
	UniquePtr<Volumetric> m_vol; ///< Volumetric effects.
	UniquePtr<Lf> m_lf; ///< Forward shading lens flares.
	UniquePtr<FsUpscale> m_fsUpscale;
	UniquePtr<DownscaleBlur> m_downscale;
	UniquePtr<Smaa> m_smaa;
	UniquePtr<Tm> m_tm;
	UniquePtr<Ssao> m_ssao;
	UniquePtr<Bloom> m_bloom;
	UniquePtr<Pps> m_pps; ///< Postprocessing rendering stage
	UniquePtr<Dbg> m_dbg; ///< Debug stage.
	/// @}

	U32 m_width;
	U32 m_height;

	F32 m_lodDistance; ///< Distance that used to calculate the LOD
	Bool8 m_tessellation;

	ShaderResourcePtr m_drawQuadVert;

	RenderableDrawer m_sceneDrawer;

	U64 m_frameCount; ///< Frame number

	U64 m_prevLoadRequestCount = 0;
	U64 m_prevAsyncTasksCompleted = 0;
	Bool m_resourcesDirty = true;

	Bool8 m_willDrawToDefaultFbo = false;

	Mat4 m_prevViewProjMat = Mat4::getIdentity();
	Mat4 m_prevCamTransform = Mat4::getIdentity();

	TexturePtr m_dummyTex;
	BufferPtr m_dummyBuff;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);

	ANKI_USE_RESULT Error buildCommandBuffers(RenderingContext& ctx);
	ANKI_USE_RESULT Error buildCommandBuffersInternal(RenderingContext& ctx, U32 threadId, PtrSize threadCount);
};
/// @}

} // end namespace anki
