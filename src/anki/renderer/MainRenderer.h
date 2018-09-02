// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/Common.h>
#include <anki/resource/Forward.h>
#include <anki/renderer/Renderer.h>

namespace anki
{

// Forward
class ResourceManager;
class ConfigSet;
class StagingGpuMemoryManager;
class UiManager;

/// @addtogroup renderer
/// @{

/// MainRenderer statistics.
class MainRendererStats : public RendererStats
{
public:
	Second m_renderingTime ANKI_DBG_NULLIFY;
};

/// Main onscreen renderer
class MainRenderer
{
public:
	MainRenderer();

	~MainRenderer();

	ANKI_USE_RESULT Error init(ThreadHive* hive,
		ResourceManager* resources,
		GrManager* gl,
		StagingGpuMemoryManager* stagingMem,
		UiManager* ui,
		AllocAlignedCallback allocCb,
		void* allocCbUserData,
		const ConfigSet& config,
		Timestamp* globTimestamp);

	ANKI_USE_RESULT Error render(RenderQueue& rqueue, TexturePtr presentTex);

	Dbg& getDbg();

	F32 getAspectRatio() const;

	const Renderer& getOffscreenRenderer() const
	{
		return *m_r;
	}

	Renderer& getOffscreenRenderer()
	{
		return *m_r;
	}

	const MainRendererStats& getStats() const
	{
		return m_stats;
	}

private:
	HeapAllocator<U8> m_alloc;
	StackAllocator<U8> m_frameAlloc;

	UniquePtr<Renderer> m_r;
	Bool8 m_rDrawToDefaultFb = false;

	ShaderProgramResourcePtr m_blitProg;
	ShaderProgramPtr m_blitGrProg;

	U32 m_width = 0; ///< Default FB size.
	U32 m_height = 0; ///< Default FB size.

	F32 m_renderingQuality = 1.0;

	RenderGraphPtr m_rgraph;
	RenderTargetDescription m_tmpRtDesc;

	MainRendererStats m_stats;

	class
	{
	public:
		const RenderingContext* m_ctx = nullptr;
		Atomic<U32> m_secondaryTaskId = {0};
	} m_runCtx;

	void runBlit(RenderPassWorkContext& rgraphCtx);
	void present(RenderPassWorkContext& rgraphCtx);

	// A RenderPassWorkCallback for blit pass.
	static void runCallback(RenderPassWorkContext& rgraphCtx)
	{
		MainRenderer* const self = scast<MainRenderer*>(rgraphCtx.m_userData);
		self->runBlit(rgraphCtx);
	}

	// A RenderPassWorkCallback for present.
	static void presentCallback(RenderPassWorkContext& rgraphCtx)
	{
		// Do nothing. This pass is dummy
	}

	static void executeSecondaryCallback(
		void* userData, U32 threadId, ThreadHive& hive, ThreadHiveSemaphore* signalSemaphore);
};
/// @}

} // end namespace anki
