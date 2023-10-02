// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/Common.h>
#include <AnKi/Resource/Forward.h>
#include <AnKi/Renderer/Renderer.h>

namespace anki {

/// @addtogroup renderer
/// @{

class MainRendererInitInfo
{
public:
	UVec2 m_swapchainSize = UVec2(0u);

	AllocAlignedCallback m_allocCallback = nullptr;
	void* m_allocCallbackUserData = nullptr;
};

/// Main onscreen renderer
class MainRenderer : public MakeSingleton<MainRenderer>
{
	template<typename>
	friend class MakeSingleton;

public:
	Error init(const MainRendererInitInfo& inf);

	Error render(Texture* presentTex);

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

private:
	StackMemoryPool m_framePool;

	Renderer* m_r = nullptr;
	Bool m_rDrawToDefaultFb = false;

	ShaderProgramResourcePtr m_blitProg;
	ShaderProgramPtr m_blitGrProg;

	UVec2 m_swapchainResolution = UVec2(0u);

	RenderGraphPtr m_rgraph;
	RenderTargetDescription m_tmpRtDesc;
	FramebufferDescription m_fbDescr;

	class
	{
	public:
		const RenderingContext* m_ctx = nullptr;
		Atomic<U32> m_secondaryTaskId = {0};
	} m_runCtx;

	MainRenderer();

	~MainRenderer();
};
/// @}

} // end namespace anki
