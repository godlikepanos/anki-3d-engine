// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Renderer/ImageStreaming.h>
#include <AnKi/GpuMemory/GpuVisibleTransientMemoryPool.h>
#include <AnKi/Resource/StreamingImageResourceManager.h>
#include <AnKi/Util/Tracer.h>

namespace anki {

Error ImageStreaming::init()
{
	ANKI_CHECK(m_clearFeedbackBufferProg.load("ShaderBinaries/ImageStreaming.ankiprogbin", {}, "ClearFeedbackBuffer"));
	ANKI_CHECK(m_storeRequestsProg.load("ShaderBinaries/ImageStreaming.ankiprogbin", {}, "StoreRequests"));

	return Error::kNone;
}

void ImageStreaming::populateRenderGraphPreRendering()
{
	ANKI_TRACE_SCOPED_EVENT(ImageStreaming);

	// Access the readbacks and make the streaming requests
	{
		DynamicArray<ImageStreamingRequest, MemoryPoolPtrWrapper<StackMemoryPool>> readbackData(&getRenderer().getFrameMemoryPool());
		getRenderer().getReadbackManager().readMostRecentData(m_readback, readbackData);
		if(readbackData.getSize())
		{
			DynamicArray<ImageStreamingRequest, MemoryPoolPtrWrapper<StackMemoryPool>> requests(&getRenderer().getFrameMemoryPool());
			requests.resize(readbackData.getSize());

			U32 requestCount = 0;
			for(const ImageStreamingRequest& req : readbackData)
			{
				if(req.m_descriptorIndex != 0)
				{
					ANKI_ASSERT(req.m_mipmap <= kImageDescriptorMaxMipmaps);
					ANKI_ASSERT(req.m_resourceUuid > 0);
					requests[requestCount++] = req;
				}
			}

			if(requestCount)
			{
				StreamingImageResourceManager::getSingleton().appendStreamingRequests(
					WeakArray<ImageStreamingRequest>(requests.getBegin(), requestCount));
			}
		}
	}

	RenderGraphBuilder& rgraph = getRenderingContext().m_renderGraphDescr;

	m_runCtx.m_feedbackBuff = GpuVisibleTransientMemoryPool::getSingleton().allocateStructuredBuffer<I32>(
		StreamingImageResourceManager::getSingleton().getMaxDescriptorIndex() + 1);
	m_runCtx.m_feedbackBuffHandle = rgraph.importBuffer(m_runCtx.m_feedbackBuff, BufferUsageBit::kNone);

	// Clear the buffer pass
	NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("Image Streaming: Clear");
	pass.newBufferDependency(m_runCtx.m_feedbackBuffHandle, BufferUsageBit::kUavCompute);

	pass.setWork([this](RenderPassWorkContext& ctx) {
		CommandBuffer& cmdb = *ctx.m_commandBuffer;

		cmdb.bindShaderProgram(m_clearFeedbackBufferProg.get());
		cmdb.bindUav(0, 0, m_runCtx.m_feedbackBuff);
		cmdb.dispatchCompute((StreamingImageResourceManager::getSingleton().getMaxDescriptorIndex() + 1 + 63) / 64, 1, 1);
	});
}

void ImageStreaming::populateRenderGraphPostRendering()
{
	ANKI_TRACE_SCOPED_EVENT(ImageStreaming);

	RenderGraphBuilder& rgraph = getRenderingContext().m_renderGraphDescr;

	const BufferView requestsBuff = getRenderer().getReadbackManager().allocateStructuredBuffer<ImageStreamingRequest>(
		m_readback, StreamingImageResourceManager::getSingleton().getMaxDescriptorIndex() + 1);

	NonGraphicsRenderPass& pass = rgraph.newNonGraphicsRenderPass("Image Streaming: Store requests");
	pass.newBufferDependency(m_runCtx.m_feedbackBuffHandle, BufferUsageBit::kSrvCompute);

	pass.setWork([this, requestsBuff](RenderPassWorkContext& ctx) {
		CommandBuffer& cmdb = *ctx.m_commandBuffer;

		cmdb.bindShaderProgram(m_storeRequestsProg.get());
		cmdb.bindSrv(0, 0, m_runCtx.m_feedbackBuff);
		cmdb.bindSrv(1, 0, StreamingImageResourceManager::getSingleton().getBuffer());
		cmdb.bindUav(0, 0, requestsBuff);

		cmdb.dispatchCompute((StreamingImageResourceManager::getSingleton().getMaxDescriptorIndex() + 1 + 63) / 64, 1, 1);
	});
}

} // end namespace anki
