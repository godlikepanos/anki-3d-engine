// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/GrManager.h>
#include <AnKi/Gr/Texture.h>
#include <AnKi/Gr/Buffer.h>
#include <AnKi/Gr/CommandBuffer.h>
#include <AnKi/Gr/Fence.h>

namespace anki {

void GrManager::commonPostInit()
{
	const U32 texDimension = 2;

	BufferInitInfo buffInit("TmpBuff");
	buffInit.m_size = sizeof(U32) * texDimension * texDimension;
	buffInit.m_mapAccess = BufferMapAccessBit::kWrite;
	buffInit.m_usage = BufferUsageBit::kAllCopy;
	BufferPtr tmpBuff = newBuffer(buffInit);

	U32* pixels = static_cast<U32*>(tmpBuff->map(0, kMaxPtrSize));
	for(U32 i = 0; i < texDimension * texDimension; ++i)
	{
		pixels[i] = (0xFFu << 24u) | (0xFFu << 16u) | 0xFFu; // Magenda
	}

	tmpBuff->unmap();

	TextureInitInfo texInit("Slot0BindlessTexture");
	texInit.m_width = texDimension;
	texInit.m_height = texDimension;
	texInit.m_format = Format::kR8G8B8A8_Unorm;
	texInit.m_usage = TextureUsageBit::kAllSrv | TextureUsageBit::kCopyDestination;
	TexturePtr tex = newTexture(texInit);
	TextureView view(tex.get());

	[[maybe_unused]] const U32 bindlessIndex = tex->getOrCreateBindlessTextureIndex(TextureSubresourceDesc::all());
	ANKI_ASSERT(bindlessIndex == 0);

	CommandBufferInitInfo cmdbInit("TmpCmdb");
	cmdbInit.m_flags = CommandBufferFlag::kGeneralWork | CommandBufferFlag::kSmallBatch;
	CommandBufferPtr cmdb = newCommandBuffer(cmdbInit);

	TextureBarrierInfo barr;
	barr.m_textureView = view;
	barr.m_previousUsage = TextureUsageBit::kNone;
	barr.m_nextUsage = TextureUsageBit::kCopyDestination;
	cmdb->setPipelineBarrier({&barr, 1}, {}, {});
	cmdb->copyBufferToTexture(BufferView(tmpBuff.get()), view);
	barr.m_previousUsage = TextureUsageBit::kCopyDestination;
	barr.m_nextUsage = TextureUsageBit::kAllSrv;
	cmdb->setPipelineBarrier({&barr, 1}, {}, {});
	cmdb->endRecording();

	FencePtr fence;
	submit(cmdb.get(), {}, &fence);

	const Bool signaled = fence->clientWait(kMaxSecond);
	if(!signaled)
	{
		ANKI_GR_LOGF("GPU timeout detected");
	}

	tex->retain();
	m_slot0BindlessTexture = tex.get();
}

void GrManager::commonPreDestroy()
{
	if(m_slot0BindlessTexture)
	{
		TexturePtr tex(m_slot0BindlessTexture);
		[[maybe_unused]] const I32 count = m_slot0BindlessTexture->release();
		ANKI_ASSERT(count == 2);
		m_slot0BindlessTexture = nullptr;
	}
}

} // end namespace anki
