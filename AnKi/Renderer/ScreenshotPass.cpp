// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/ScreenshotPass.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Util/Filesystem.h>
#include <AnKi/Resource/Stb.h>

namespace anki {

ScreenshotPass::~ScreenshotPass()
{
	for(BufferPtr& buff : m_buffers)
	{
		if(buff)
		{
			buff->unmap();
		}
	}
}

Error ScreenshotPass::init()
{
	ANKI_CHECK(m_prog.load("ShaderBinaries/ScreenshotPass.ankiprogbin", {}, "", ShaderTypeBit::kCompute));
	return Error::kNone;
}

void ScreenshotPass::populateRenderGraph()
{
	ANKI_TRACE_SCOPED_EVENT(Screenshot);

	RenderGraphBuilder& rgraph = getRenderingContext().m_renderGraphDescr;
	const Bool bCaptureScreenshot = m_framesToCaptureCount > 0;

	// Store screenshots
	for(U32 f = 0; f < kFramesInFlightCount; ++f)
	{
		FencePtr& fence = m_fences[f];

		if(!fence) [[likely]]
		{
			continue;
		}

		if(bCaptureScreenshot && f == m_localFrame)
		{
			// This frame will be reused, need to wait it
			const Bool signaled = fence->clientWait(kMaxSecond);
			if(!signaled)
			{
				ANKI_R_LOGF("GPU timeout detected");
			}
		}

		if(fence->signaled())
		{
			fence.reset(nullptr);

			const Error err = saveTexture(f);
			if(err)
			{
				// Ignore it
			}
		}
	}

	if(!bCaptureScreenshot) [[likely]]
	{
		return;
	}

	m_rendererFrames[m_localFrame] = getRenderer().getFrameCount();

	if(!m_buffers[m_localFrame])
	{
		// Lazy init

		BufferInitInfo inf("Screenshot");
		inf.m_size = getRenderer().getSwapchainResolution().x * getRenderer().getSwapchainResolution().y * sizeof(U32);
		inf.m_mapAccess = BufferMapAccessBit::kRead;
		inf.m_usage = BufferUsageBit::kUavCompute;

		m_buffers[m_localFrame] = GrManager::getSingleton().newBuffer(inf);

		m_mappedMemories[m_localFrame] = m_buffers[m_localFrame]->map(0, kMaxPtrSize);
	}

	// Create the pass
	{
		NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("Screenshot");

		pass.newTextureDependency(getRenderingContext().m_swapchainRenderTarget, TextureUsageBit::kSrvCompute);

		pass.setWork([this, frame = m_localFrame](RenderPassWorkContext& rgraphCtx) {
			CommandBuffer& cmdb = *rgraphCtx.m_commandBuffer;

			cmdb.bindShaderProgram(m_prog.get());

			rgraphCtx.bindSrv(0, 0, getRenderingContext().m_swapchainRenderTarget);
			cmdb.bindUav(0, 0, BufferView(m_buffers[frame].get()));

			dispatchPPCompute(cmdb, 8, 8, getRenderer().getSwapchainResolution().x, getRenderer().getSwapchainResolution().y);
		});
	}

	// Create a dummy pass to transition the swapchain
	{
		GraphicsRenderPass& pass = rgraph.newGraphicsRenderPass("ScreenshotDummyTransition");
		pass.newTextureDependency(getRenderingContext().m_swapchainRenderTarget, TextureUsageBit::kRtvDsvWrite);

		pass.setWork([]([[maybe_unused]] RenderPassWorkContext& rgraphCtx) {});
	}
}

void ScreenshotPass::setFence(Fence* fence)
{
	ANKI_ASSERT(fence);

	const Bool bCaptureScreenshot = m_framesToCaptureCount > 0;

	if(bCaptureScreenshot) [[unlikely]]
	{
		ANKI_ASSERT(!m_fences[m_localFrame]);
		m_fences[m_localFrame].reset(fence);

		// End
		m_localFrame = (m_localFrame + 1) % kFramesInFlightCount;
		--m_framesToCaptureCount;
	}
}

Error ScreenshotPass::saveTexture(U32 localFrame) const
{
	ANKI_ASSERT(!m_fences[localFrame]);
	ANKI_ASSERT(m_mappedMemories[localFrame]);

	const U32 width = getRenderer().getSwapchainResolution().x;
	const U32 height = getRenderer().getSwapchainResolution().y;

	String tempDir;
	ANKI_CHECK(getTempDirectory(tempDir));

	RendererString filename;
	filename.sprintf("%s/Screenshot%" PRIu64 ".png", tempDir.cstr(), m_rendererFrames[localFrame]);

	const I32 ok = stbi_write_png(filename.cstr(), width, height, 4, m_mappedMemories[localFrame], 0);
	if(!ok)
	{
		ANKI_R_LOGE("Failed to write screenshot: %s", filename.cstr());
		return Error::kFunctionFailed;
	}

	ANKI_R_LOGI("Stored screenshot: %s", filename.cstr());

	return Error::kNone;
}

} // end namespace anki
