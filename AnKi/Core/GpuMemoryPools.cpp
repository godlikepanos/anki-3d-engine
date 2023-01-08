// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Core/GpuMemoryPools.h>
#include <AnKi/Core/ConfigSet.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Gr/CommandBuffer.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Resource/ResourceManager.h>

namespace anki {

void UnifiedGeometryMemoryPool::init(HeapMemoryPool* pool, GrManager* gr, const ConfigSet& cfg)
{
	ANKI_ASSERT(pool && gr);

	const PtrSize poolSize = cfg.getCoreGlobalVertexMemorySize();

	const Array classes = {1_KB, 8_KB, 32_KB, 128_KB, 512_KB, 4_MB, 8_MB, 16_MB, poolSize};

	BufferUsageBit buffUsage = BufferUsageBit::kVertex | BufferUsageBit::kIndex | BufferUsageBit::kTransferDestination
							   | (BufferUsageBit::kAllTexture & BufferUsageBit::kAllRead);

	if(gr->getDeviceCapabilities().m_rayTracingEnabled)
	{
		buffUsage |= BufferUsageBit::kAccelerationStructureBuild;
	}

	m_pool.init(gr, pool, buffUsage, classes, poolSize, "UnifiedGeometry", false);

	// Allocate something dummy to force creating the GPU buffer
	SegregatedListsGpuMemoryPoolToken token;
	allocate(16, 4, token);
	free(token);
}

void GpuSceneMemoryPool::init(HeapMemoryPool* pool, GrManager* gr, const ConfigSet& cfg)
{
	ANKI_ASSERT(pool && gr);

	const PtrSize poolSize = cfg.getCoreGpuSceneInitialSize();

	const Array classes = {32_B, 64_B, 128_B, 256_B, poolSize};

	BufferUsageBit buffUsage = BufferUsageBit::kAllStorage | BufferUsageBit::kTransferDestination;

	m_pool.init(gr, pool, buffUsage, classes, poolSize, "GpuScene", true);
}

RebarStagingGpuMemoryPool::~RebarStagingGpuMemoryPool()
{
	GrManager& gr = m_buffer->getManager();

	gr.finish();

	m_buffer->unmap();
	m_buffer.reset(nullptr);
}

Error RebarStagingGpuMemoryPool::init(GrManager* gr, const ConfigSet& cfg)
{
	BufferInitInfo buffInit("ReBar");
	buffInit.m_mapAccess = BufferMapAccessBit::kWrite;
	buffInit.m_size = cfg.getCoreRebarGpuMemorySize();
	buffInit.m_usage =
		BufferUsageBit::kAllUniform | BufferUsageBit::kAllStorage | BufferUsageBit::kVertex | BufferUsageBit::kIndex;
	m_buffer = gr->newBuffer(buffInit);

	m_bufferSize = buffInit.m_size;

	m_alignment = gr->getDeviceCapabilities().m_uniformBufferBindOffsetAlignment;
	m_alignment = max(m_alignment, gr->getDeviceCapabilities().m_storageBufferBindOffsetAlignment);
	m_alignment = max(m_alignment, gr->getDeviceCapabilities().m_sbtRecordAlignment);

	m_mappedMem = static_cast<U8*>(m_buffer->map(0, kMaxPtrSize, BufferMapAccessBit::kWrite));

	return Error::kNone;
}

void* RebarStagingGpuMemoryPool::allocateFrame(PtrSize size, RebarGpuMemoryToken& token)
{
	void* address = tryAllocateFrame(size, token);
	if(ANKI_UNLIKELY(address == nullptr))
	{
		ANKI_CORE_LOGF("Out of ReBAR GPU memory");
	}

	return address;
}

void* RebarStagingGpuMemoryPool::tryAllocateFrame(PtrSize origSize, RebarGpuMemoryToken& token)
{
	const PtrSize size = getAlignedRoundUp(m_alignment, origSize);

	// Try in a loop because we may end up with an allocation its offset crosses the buffer's end
	PtrSize offset;
	Bool done = false;
	do
	{
		offset = m_offset.fetchAdd(size) % m_bufferSize;
		const PtrSize end = (offset + origSize) % (m_bufferSize + 1);

		done = offset < end;
	} while(!done);

	void* address = m_mappedMem + offset;
	token.m_offset = offset;
	token.m_range = origSize;

	return address;
}

PtrSize RebarStagingGpuMemoryPool::endFrame()
{
	const PtrSize crntOffset = m_offset.getNonAtomically();

	const PtrSize usedMemory = crntOffset - m_previousFrameEndOffset;
	m_previousFrameEndOffset = crntOffset;

	if(usedMemory >= PtrSize(0.8 * F64(m_bufferSize / kMaxFramesInFlight)))
	{
		ANKI_CORE_LOGW("Frame used more that 80%% of its safe limit of ReBAR memory");
	}

	ANKI_TRACE_INC_COUNTER(ReBarUsedMemory, usedMemory);
	return usedMemory;
}

/// It packs the source and destination offsets as well as the size of the patch itself.
class GpuSceneMicroPatcher::PatchHeader
{
public:
	U32 m_dwordCountAndSrcDwordOffsetPack;
	U32 m_dstDwordOffset;
};

GpuSceneMicroPatcher::~GpuSceneMicroPatcher()
{
	static_assert(sizeof(PatchHeader) == 8);
}

Error GpuSceneMicroPatcher::init(ResourceManager* rsrc)
{
	ANKI_CHECK(rsrc->loadResource("ShaderBinaries/GpuSceneMicroPatching.ankiprogbin", m_copyProgram));
	const ShaderProgramResourceVariant* variant;
	m_copyProgram->getOrCreateVariant(variant);
	m_grProgram = variant->getProgram();

	return Error::kNone;
}

void GpuSceneMicroPatcher::newCopy(StackMemoryPool& frameCpuPool, PtrSize gpuSceneDestOffset, PtrSize dataSize,
								   const void* data)
{
	ANKI_ASSERT(dataSize > 0 && (dataSize % 4) == 0);
	ANKI_ASSERT((ptrToNumber(data) % 4) == 0);
	ANKI_ASSERT((gpuSceneDestOffset % 4) == 0 && gpuSceneDestOffset / 4 < kMaxU32);

	const U32 dataDwords = U32(dataSize / 4);
	U32 gpuSceneDestDwordOffset = U32(gpuSceneDestOffset / 4);

	const U32* patchIt = static_cast<const U32*>(data);
	const U32* const patchEnd = patchIt + dataDwords;

	// Break the data into multiple copies
	LockGuard lock(m_mtx);
	while(patchIt < patchEnd)
	{
		const U32 patchDwords = min(kDwordsPerPatch, U32(patchEnd - patchIt));

		PatchHeader& header = *m_crntFramePatchHeaders.emplaceBack(frameCpuPool);
		ANKI_ASSERT(((patchDwords - 1) & 0b111111) == (patchDwords - 1));
		header.m_dwordCountAndSrcDwordOffsetPack = patchDwords - 1;
		header.m_dwordCountAndSrcDwordOffsetPack <<= 26;
		ANKI_ASSERT((m_crntFramePatchData.getSize() & 0x3FFFFFF) == m_crntFramePatchData.getSize());
		header.m_dwordCountAndSrcDwordOffsetPack |= m_crntFramePatchData.getSize();
		header.m_dstDwordOffset = gpuSceneDestDwordOffset;

		const U32 srcOffset = m_crntFramePatchData.getSize();
		m_crntFramePatchData.resize(frameCpuPool, srcOffset + patchDwords);
		memcpy(&m_crntFramePatchData[srcOffset], patchIt, patchDwords * 4);

		patchIt += patchDwords;
		gpuSceneDestDwordOffset += patchDwords;
	}
}

void GpuSceneMicroPatcher::patchGpuScene(RebarStagingGpuMemoryPool& rebarPool, CommandBuffer& cmdb,
										 const BufferPtr& gpuSceneBuffer)
{
	if(m_crntFramePatchHeaders.getSize() == 0)
	{
		return;
	}

	ANKI_ASSERT(m_crntFramePatchData.getSize() > 0);

	ANKI_TRACE_INC_COUNTER(GpuSceneMicroPatches, m_crntFramePatchHeaders.getSize());
	ANKI_TRACE_INC_COUNTER(GpuSceneMicroPatchUploadData, m_crntFramePatchData.getSizeInBytes());

	RebarGpuMemoryToken headersToken;
	void* mapped = rebarPool.allocateFrame(m_crntFramePatchHeaders.getSizeInBytes(), headersToken);
	memcpy(mapped, &m_crntFramePatchHeaders[0], m_crntFramePatchHeaders.getSizeInBytes());

	RebarGpuMemoryToken dataToken;
	mapped = rebarPool.allocateFrame(m_crntFramePatchData.getSizeInBytes(), dataToken);
	memcpy(mapped, &m_crntFramePatchData[0], m_crntFramePatchData.getSizeInBytes());

	cmdb.bindStorageBuffer(0, 0, rebarPool.getBuffer(), headersToken.m_offset, headersToken.m_range);
	cmdb.bindStorageBuffer(0, 1, rebarPool.getBuffer(), dataToken.m_offset, dataToken.m_range);
	cmdb.bindStorageBuffer(0, 2, gpuSceneBuffer, 0, kMaxPtrSize);

	cmdb.bindShaderProgram(m_grProgram);

	const U32 workgroupCountX = m_crntFramePatchHeaders.getSize();
	cmdb.dispatchCompute(workgroupCountX, 1, 1);

	// Cleanup to prepare for the new frame
	U32* data;
	U32 size, storage;
	m_crntFramePatchData.moveAndReset(data, size, storage);
	PatchHeader* datah;
	m_crntFramePatchHeaders.moveAndReset(datah, size, storage);
}

} // end namespace anki
