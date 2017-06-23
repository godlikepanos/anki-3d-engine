// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/Common.h>
#include <anki/renderer/Drawer.h>
#include <anki/Math.h>
#include <anki/Gr.h>
#include <anki/resource/Forward.h>
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

/// @addtogroup renderer
/// @{

/// Rendering context.
class RenderingContext
{
public:
	RenderQueue* m_renderQueue ANKI_DBG_NULLIFY;

	// Extra matrices
	Mat4 m_projMatJitter;
	Mat4 m_viewProjMatJitter;
	Mat4 m_jitterMat;
	Mat4 m_prevViewProjMat;
	Mat4 m_prevCamTransform;

	Vec4 m_unprojParams;

	CommandBufferPtr m_commandBuffer; ///< Primary command buffer.
	CommandBufferPtr m_defaultFbCommandBuffer; ///< The default framebuffer renderpass is in a separate cmdb.

	StackAllocator<U8> m_tempAllocator;

	class GBuffer
	{
	public:
		Array<CommandBufferPtr, ThreadPool::MAX_THREADS> m_commandBuffers;
		U32 m_lastThreadWithWork = 0;
	} m_gbuffer;

	class LensFlare
	{
	public:
		DynamicArrayAuto<OcclusionQueryPtr> m_queriesToTest;

		LensFlare(const StackAllocator<U8>& alloc)
			: m_queriesToTest(alloc)
		{
		}
	} m_lensFlare;

	class LightShading
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
	} m_lightShading;

	class ShadowMapping
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

		ShadowMapping(const StackAllocator<U8>& alloc)
			: m_spotFramebuffers(alloc)
			, m_omniFramebuffers(alloc)
			, m_spotCacheIndices(alloc)
			, m_omniCacheIndices(alloc)
			, m_spotCommandBuffers(alloc)
			, m_omniCommandBuffers(alloc)
		{
		}
	} m_shadowMapping;

	class ForwardShading
	{
	public:
		Array<CommandBufferPtr, ThreadPool::MAX_THREADS> m_commandBuffers;
		U32 m_lastThreadWithWork = 0;
	} m_forwardShading;

	FramebufferPtr m_outFb;
	U32 m_outFbWidth = 0;
	U32 m_outFbHeight = 0;

	RenderingContext(const StackAllocator<U8>& alloc)
		: m_tempAllocator(alloc)
		, m_lensFlare(alloc)
		, m_shadowMapping(alloc)
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

	Indirect& getIndirect()
	{
		return *m_indirect;
	}

	ShadowMapping& getShadowMapping()
	{
		return *m_shadowMapping;
	}

	GBuffer& getGBuffer()
	{
		return *m_gbuffer;
	}

	LightShading& getLightShading()
	{
		return *m_lightShading;
	}

	DepthDownscale& getDepthDownscale()
	{
		return *m_depth;
	}

	ForwardShading& getForwardShading()
	{
		return *m_forwardShading;
	}

	Volumetric& getVolumetric()
	{
		return *m_vol;
	}

	Tonemapping& getTonemapping()
	{
		return *m_tonemapping;
	}

	Ssao& getSsao()
	{
		return *m_ssao;
	}

	Bloom& getBloom()
	{
		return *m_bloom;
	}

	FinalComposite& getFinalComposite()
	{
		return *m_finalComposite;
	}

	Dbg& getDbg()
	{
		return *m_dbg;
	}

	TemporalAA& getTemporalAA()
	{
		return *m_temporalAA;
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
	U calculateLod(F32 distance) const
	{
		ANKI_ASSERT(m_lodDistances.getSize() == 2);
		if(distance < m_lodDistances[0])
		{
			return 0;
		}
		else if(distance < m_lodDistances[1])
		{
			return 1;
		}
		else
		{
			return 2;
		}
	}

	/// Create the init info for a 2D texture that will be used as a render target.
	ANKI_USE_RESULT TextureInitInfo create2DRenderTargetInitInfo(U32 w,
		U32 h,
		const PixelFormat& format,
		TextureUsageBit usage,
		SamplingFilter filter,
		U mipsCount = 1,
		CString name = {});

	ANKI_USE_RESULT TexturePtr createAndClearRenderTarget(const TextureInitInfo& inf);

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

	SamplerPtr getNearestSampler() const
	{
		return m_nearestSampler;
	}

	SamplerPtr getLinearSampler() const
	{
		return m_linearSampler;
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
	UniquePtr<Indirect> m_indirect;
	UniquePtr<ShadowMapping> m_shadowMapping; ///< Shadow mapping.
	UniquePtr<GBuffer> m_gbuffer; ///< Material rendering stage
	UniquePtr<LightShading> m_lightShading; ///< Illumination rendering stage
	UniquePtr<DepthDownscale> m_depth;
	UniquePtr<ForwardShading> m_forwardShading; ///< Forward shading.
	UniquePtr<Volumetric> m_vol; ///< Volumetric effects.
	UniquePtr<LensFlare> m_lensFlare; ///< Forward shading lens flares.
	UniquePtr<ForwardShadingUpscale> m_fsUpscale;
	UniquePtr<DownscaleBlur> m_downscale;
	UniquePtr<TemporalAA> m_temporalAA;
	UniquePtr<Tonemapping> m_tonemapping;
	UniquePtr<Ssao> m_ssao;
	UniquePtr<Bloom> m_bloom;
	UniquePtr<FinalComposite> m_finalComposite; ///< Postprocessing rendering stage
	UniquePtr<Dbg> m_dbg; ///< Debug stage.
	/// @}

	U32 m_width;
	U32 m_height;

	Array<F32, MAX_LOD_COUNT - 1> m_lodDistances; ///< Distance that used to calculate the LOD
	Bool8 m_tessellation;

	RenderableDrawer m_sceneDrawer;

	U64 m_frameCount; ///< Frame number

	U64 m_prevLoadRequestCount = 0;
	U64 m_prevAsyncTasksCompleted = 0;
	Bool m_resourcesDirty = true;

	Bool8 m_willDrawToDefaultFbo = false;

	Mat4 m_prevViewProjMat = Mat4::getIdentity();
	Mat4 m_prevCamTransform = Mat4::getIdentity();

	Array<Mat4, 16> m_jitteredMats16x;
	Array<Mat4, 8> m_jitteredMats8x;

	TexturePtr m_dummyTex;
	BufferPtr m_dummyBuff;

	SamplerPtr m_nearestSampler;
	SamplerPtr m_linearSampler;

	ANKI_USE_RESULT Error initInternal(const ConfigSet& initializer);

	ANKI_USE_RESULT Error buildCommandBuffers(RenderingContext& ctx);
	void buildCommandBuffersInternal(RenderingContext& ctx, U32 threadId, PtrSize threadCount);

	void initJitteredMats();
};
/// @}

} // end namespace anki
