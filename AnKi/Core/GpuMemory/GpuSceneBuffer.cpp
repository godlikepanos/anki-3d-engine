// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Core/GpuMemory/GpuSceneBuffer.h>
#include <AnKi/Core/GpuMemory/RebarTransientMemoryPool.h>
#include <AnKi/Core/CVarSet.h>
#include <AnKi/Core/StatsSet.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Gr/CommandBuffer.h>

namespace anki {

static StatCounter g_gpuSceneBufferAllocatedSizeStatVar(StatCategory::kGpuMem, "GPU scene allocated",
														StatFlag::kBytes | StatFlag::kMainThreadUpdates);
static StatCounter g_gpuSceneBufferTotalStatVar(StatCategory::kGpuMem, "GPU scene total", StatFlag::kBytes | StatFlag::kMainThreadUpdates);
static StatCounter g_gpuSceneBufferFragmentationStatVar(StatCategory::kGpuMem, "GPU scene fragmentation",
														StatFlag::kFloat | StatFlag::kMainThreadUpdates);

static NumericCVar<PtrSize> g_gpuSceneInitialSizeCVar(CVarSubsystem::kCore, "GpuSceneInitialSize", 64_MB, 16_MB, 2_GB,
													  "Global memory for the GPU scene");

void GpuSceneBuffer::init()
{
	const PtrSize poolSize = g_gpuSceneInitialSizeCVar.get();

	const Array classes = {32_B, 64_B, 128_B, 256_B, poolSize};

	BufferUsageBit buffUsage = BufferUsageBit::kAllUav | BufferUsageBit::kTransferDestination;

	m_pool.init(buffUsage, classes, poolSize, "GpuScene", true);

	// Allocate something dummy to force creating the GPU buffer
	GpuSceneBufferAllocation alloc = allocate(16, 4);
	deferredFree(alloc);
}

void GpuSceneBuffer::updateStats() const
{
	F32 externalFragmentation;
	PtrSize userAllocatedSize, totalSize;
	m_pool.getStats(externalFragmentation, userAllocatedSize, totalSize);

	g_gpuSceneBufferAllocatedSizeStatVar.set(userAllocatedSize);
	g_gpuSceneBufferTotalStatVar.set(totalSize);
	g_gpuSceneBufferFragmentationStatVar.set(externalFragmentation);
}

/// It packs the source and destination offsets as well as the size of the patch itself.
class GpuSceneMicroPatcher::PatchHeader
{
public:
	U32 m_dwordCountAndSrcDwordOffsetPack;
	U32 m_dstDwordOffset;
};

GpuSceneMicroPatcher::GpuSceneMicroPatcher()
{
}

GpuSceneMicroPatcher::~GpuSceneMicroPatcher()
{
	static_assert(sizeof(PatchHeader) == 8);
}

Error GpuSceneMicroPatcher::init()
{
	ANKI_CHECK(ResourceManager::getSingleton().loadResource("ShaderBinaries/GpuSceneMicroPatching.ankiprogbin", m_copyProgram));
	const ShaderProgramResourceVariant* variant;
	m_copyProgram->getOrCreateVariant(variant);
	m_grProgram.reset(&variant->getProgram());

	return Error::kNone;
}

void GpuSceneMicroPatcher::newCopy(StackMemoryPool& frameCpuPool, PtrSize gpuSceneDestOffset, PtrSize dataSize, const void* data)
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

	if(m_crntFramePatchHeaders.getSize() == 0)
	{
		m_crntFramePatchHeaders = DynamicArray<PatchHeader, MemoryPoolPtrWrapper<StackMemoryPool>>(&frameCpuPool);
		m_crntFramePatchData = DynamicArray<U32, MemoryPoolPtrWrapper<StackMemoryPool>>(&frameCpuPool);
	}

	while(patchIt < patchEnd)
	{
		const U32 patchDwords = min(kDwordsPerPatch, U32(patchEnd - patchIt));

		PatchHeader& header = *m_crntFramePatchHeaders.emplaceBack();
		ANKI_ASSERT(((patchDwords - 1) & 0b111111) == (patchDwords - 1));
		header.m_dwordCountAndSrcDwordOffsetPack = patchDwords - 1;
		header.m_dwordCountAndSrcDwordOffsetPack <<= 26;
		ANKI_ASSERT((m_crntFramePatchData.getSize() & 0x3FFFFFF) == m_crntFramePatchData.getSize());
		header.m_dwordCountAndSrcDwordOffsetPack |= m_crntFramePatchData.getSize();
		header.m_dstDwordOffset = gpuSceneDestDwordOffset;

		const U32 srcOffset = m_crntFramePatchData.getSize();
		m_crntFramePatchData.resize(srcOffset + patchDwords);
		memcpy(&m_crntFramePatchData[srcOffset], patchIt, patchDwords * 4);

		patchIt += patchDwords;
		gpuSceneDestDwordOffset += patchDwords;
	}
}

void GpuSceneMicroPatcher::patchGpuScene(CommandBuffer& cmdb)
{
	if(m_crntFramePatchHeaders.getSize() == 0)
	{
		return;
	}

	ANKI_ASSERT(m_crntFramePatchData.getSize() > 0);

	ANKI_TRACE_INC_COUNTER(GpuSceneMicroPatches, m_crntFramePatchHeaders.getSize());
	ANKI_TRACE_INC_COUNTER(GpuSceneMicroPatchUploadData, m_crntFramePatchData.getSizeInBytes());

	void* mapped;
	const RebarAllocation headersToken = RebarTransientMemoryPool::getSingleton().allocateFrame(m_crntFramePatchHeaders.getSizeInBytes(), mapped);
	memcpy(mapped, &m_crntFramePatchHeaders[0], m_crntFramePatchHeaders.getSizeInBytes());

	const RebarAllocation dataToken = RebarTransientMemoryPool::getSingleton().allocateFrame(m_crntFramePatchData.getSizeInBytes(), mapped);
	memcpy(mapped, &m_crntFramePatchData[0], m_crntFramePatchData.getSizeInBytes());

	cmdb.bindUavBuffer(0, 0, headersToken);
	cmdb.bindUavBuffer(0, 1, dataToken);
	cmdb.bindUavBuffer(0, 2, &GpuSceneBuffer::getSingleton().getBuffer(), 0, kMaxPtrSize);

	cmdb.bindShaderProgram(m_grProgram.get());

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
