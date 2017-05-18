// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/TransferGpuAllocator.h>
#include <anki/gr/Fence.h>
#include <anki/gr/Buffer.h>
#include <anki/gr/GrManager.h>

namespace anki
{

class TransferGpuAllocator::Memory : public ClassGpuAllocatorMemory
{
public:
	BufferPtr m_buffer;
	void* m_mappedMemory;

	DynamicArray<FencePtr> m_fences;
	U32 m_fencesCount = 0;
};

class TransferGpuAllocator::ClassInf
{
public:
	PtrSize m_slotSize;
	PtrSize m_chunkSize;
};

static const Array<TransferGpuAllocator::ClassInf, 3> CLASSES = {{{8_MB, 128_MB}, {64_MB, 256_MB}, {128_MB, 256_MB}}};

class TransferGpuAllocator::Interface : public ClassGpuAllocatorInterface
{
public:
	GrManager* m_gr;
	ResourceAllocator<U8> m_alloc;

	Error allocate(U classIdx, ClassGpuAllocatorMemory*& mem) override
	{
		TransferGpuAllocator::Memory* mm = m_alloc.newInstance<TransferGpuAllocator::Memory>();

		const PtrSize size = CLASSES[classIdx].m_chunkSize;
		mm->m_buffer = m_gr->newInstance<Buffer>(size, BufferUsageBit::BUFFER_UPLOAD_SOURCE, BufferMapAccessBit::WRITE);

		// TODO

		mem = mm;

		return ErrorCode::NONE;
	}

	void free(ClassGpuAllocatorMemory* mem) override
	{
		ANKI_ASSERT(mem);

		TransferGpuAllocator::Memory* mm = static_cast<TransferGpuAllocator::Memory*>(mem);
		mm->m_fences.destroy(m_alloc);
		mm->m_buffer.reset(nullptr);
		mm->m_fencesCount = 0;
	}

	U getClassCount() const override
	{
		return CLASSES.getSize();
	}

	void getClassInfo(U classIdx, PtrSize& slotSize, PtrSize& chunkSize) const override
	{
		slotSize = CLASSES[classIdx].m_slotSize;
		chunkSize = CLASSES[classIdx].m_chunkSize;
	}
};

} // end namespace anki