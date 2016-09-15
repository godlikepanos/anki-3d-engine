// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/common/ClassAllocator.h>
#include <tests/framework/Framework.h>

namespace anki
{

class Mem : public ClassAllocatorMemory
{
public:
	void* m_mem = nullptr;
};

class Interface final : public ClassAllocatorInterface
{
public:
	GenericMemoryPoolAllocator<U8> m_alloc;
	using Class = std::pair<PtrSize, PtrSize>;
	std::vector<Class> m_classes;

	void init()
	{
		m_classes.push_back(Class(256, 16 * 1024));
		m_classes.push_back(Class(4 * 1024, 256 * 1024));
		m_classes.push_back(Class(128 * 1024, 8 * 1024 * 1024));
		m_classes.push_back(Class(1 * 1024 * 1024, 32 * 1024 * 1024));
		m_classes.push_back(Class(10 * 1024 * 1024, 80 * 1024 * 1024));
		m_classes.push_back(Class(80 * 1024 * 1024, 160 * 1024 * 1024));
	}

	ANKI_USE_RESULT Error allocate(U classIdx, ClassAllocatorMemory*& mem)
	{
		Mem* m = m_alloc.newInstance<Mem>();

		PtrSize alignment = 256;
		m->m_mem = m_alloc.allocate(m_classes[classIdx].second, &alignment);
		mem = m;

		return ErrorCode::NONE;
	}

	void free(ClassAllocatorMemory* mem)
	{
		Mem* m = static_cast<Mem*>(mem);
		m_alloc.deallocate(m->m_mem, 0);
		m_alloc.deleteInstance(m);
	}

	U getClassCount() const
	{
		return m_classes.size();
	}

	void getClassInfo(U classIdx, PtrSize& slotSize, PtrSize& chunkSize) const
	{
		slotSize = m_classes[classIdx].first;
		slotSize = m_classes[classIdx].second;
	}
};

ANKI_TEST(Gr, ClassAllocator)
{
	HeapAllocator<U8> alloc(allocAligned, nullptr);
	Interface iface;
	iface.m_alloc = alloc;
}

} // end namespace anki
