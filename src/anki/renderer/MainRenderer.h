// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
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
	Second m_renderingCpuTime ANKI_DEBUG_CODE(= -1.0);
	Second m_renderingGpuTime ANKI_DEBUG_CODE(= -1.0);
	Second m_renderingGpuSubmitTimestamp ANKI_DEBUG_CODE(= -1.0);
};

/// Main onscreen renderer
class MainRenderer
{
public:
	MainRenderer();

	~MainRenderer();

	ANKI_USE_RESULT Error init(ThreadHive* hive, ResourceManager* resources, GrManager* gl,
							   StagingGpuMemoryManager* stagingMem, UiManager* ui, AllocAlignedCallback allocCb,
							   void* allocCbUserData, const ConfigSet& config, Timestamp* globTimestamp);

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

	void setStatsEnabled(Bool enabled)
	{
		m_statsEnabled = enabled;
	}

	const MainRendererStats& getStats() const
	{
		return m_stats;
	}

private:
	HeapAllocator<U8> m_alloc;
	StackAllocator<U8> m_frameAlloc;

	UniquePtr<Renderer> m_r;
	Bool m_rDrawToDefaultFb = false;

	ShaderProgramResourcePtr m_blitProg;
	ShaderProgramPtr m_blitGrProg;

	U32 m_width = 0; ///< Default FB size.
	U32 m_height = 0; ///< Default FB size.

	F32 m_renderingQuality = 1.0;

	RenderGraphPtr m_rgraph;
	RenderTargetDescription m_tmpRtDesc;

	MainRendererStats m_stats;
	Bool m_statsEnabled = false;

	class
	{
	public:
		const RenderingContext* m_ctx = nullptr;
		Atomic<U32> m_secondaryTaskId = {0};
	} m_runCtx;

	void runBlit(RenderPassWorkContext& rgraphCtx);
	void present(RenderPassWorkContext& rgraphCtx);
};
/// @}

} // end namespace anki
