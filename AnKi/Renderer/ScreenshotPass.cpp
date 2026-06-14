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
	for(U32 f = 0; f < kFramesInFlightCount; ++f)
	{
		if(m_fences[f])
		{
			const Bool signaled = m_fences[f]->clientWait(kMaxSecond);
			if(!signaled)
			{
				ANKI_R_LOGF("GPU timeout detected");
			}

			newTask(m_rendererFrames[f], m_mappedMemories[f]);
		}
	}

	{
		LockGuard lock(m_mtx);
		m_quit = true;
	}

	m_cvar.notifyOne();
	[[maybe_unused]] const Error err = m_thread.join();

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
	m_thread.start(this, [](ThreadCallbackInfo& inf) -> Error {
		ScreenshotPass& self = *static_cast<ScreenshotPass*>(inf.m_userData);
		return self.threadWork();
	});

	ANKI_CHECK(m_prog.load("ShaderBinaries/ScreenshotPass.ankiprogbin", {}, "", ShaderTypeBit::kCompute));

	return Error::kNone;
}

void ScreenshotPass::newTask(U64 frame, void* pixels)
{
	const PtrSize bufferSize = getRenderer().getSwapchainResolution().x * getRenderer().getSwapchainResolution().y * sizeof(U32);
	const PtrSize size = sizeof(Task) - sizeof(Task::m_pixels) + bufferSize;
	Task* task = static_cast<Task*>(RendererMemoryPool::getSingleton().allocate(size, alignof(Task)));

	task->m_frame = frame;
	memcpy(&task->m_pixels[0], pixels, bufferSize);

	LockGuard lock(m_mtx);
	m_tasks.emplaceBack(task);
	m_cvar.notifyOne();
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
			newTask(m_rendererFrames[f], m_mappedMemories[f]);
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

Error ScreenshotPass::saveTexture(U64 frame, void* pixels)
{
	ANKI_ASSERT(pixels);

	const U32 width = getRenderer().getSwapchainResolution().x;
	const U32 height = getRenderer().getSwapchainResolution().y;

	String tempDir;
	ANKI_CHECK(getTempDirectory(tempDir));

	RendererString filename;
	filename.sprintf("%s/Screenshot%" PRIu64 ".png", tempDir.cstr(), frame);

	const I32 ok = stbi_write_png(filename.cstr(), width, height, 4, pixels, 0);
	if(!ok)
	{
		ANKI_R_LOGE("Failed to write screenshot: %s", filename.cstr());
		return Error::kFunctionFailed;
	}

	ANKI_R_LOGI("Stored screenshot: %s", filename.cstr());

	return Error::kNone;
}

Error ScreenshotPass::threadWork()
{
	Bool quit = false;
	while(!quit)
	{
		RendererDynamicArray<Task*> tasks;
		{
			LockGuard lock(m_mtx);

			while(!m_quit && m_tasks.getSize() == 0)
			{
				m_cvar.wait(m_mtx);
			}

			quit = m_quit;
			tasks = std::move(m_tasks);
		}

		for(Task* task : tasks)
		{
			[[maybe_unused]] const Error err = saveTexture(task->m_frame, &task->m_pixels[0]);
			RendererMemoryPool::getSingleton().free(task);
		}
	}

	return Error::kNone;
}

} // end namespace anki
