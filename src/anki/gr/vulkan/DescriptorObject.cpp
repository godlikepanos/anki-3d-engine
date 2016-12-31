// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/DescriptorObject.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/vulkan/GrManagerImpl.h>

namespace anki
{

DescriptorObject::DescriptorObject(GrManager* manager)
	: VulkanObject(manager)
{
	m_tombstone = manager->getImplementation().getDescriptorObjectTombstoneGenerator().newTombstone();
}

DescriptorObject::~DescriptorObject()
{
	m_tombstone.killObject();
}

DescriptorObjectTombstoneGenerator::~DescriptorObjectTombstoneGenerator()
{
	auto it = m_blocks.getBegin();
	auto end = m_blocks.getEnd();
	while(it != end)
	{
		DescriptorTombstoneBlock* block = &(*it);
		++it;
		m_alloc.deleteInstance(block);
	}
}

DescriptorObjectTombstone DescriptorObjectTombstoneGenerator::newTombstone()
{
	LockGuard<SpinLock> lock(m_blocksLock);

	DescriptorTombstoneBlock* block;
	if((m_tombstoneCount % DESCRIPTORS_PER_TOMBSTONE) != 0)
	{
		// Can use the last block
		block = &m_blocks.getBack();
	}
	else
	{
		// Need new block

		block = m_alloc.newInstance<DescriptorTombstoneBlock>();
		memset(&block->m_descriptors[0], 0, sizeof(block->m_descriptors[0]));
		m_blocks.pushBack(block);
	}

	U idx = m_tombstoneCount % DESCRIPTORS_PER_TOMBSTONE;
	++m_tombstoneCount;

	block->m_descriptors[idx].set(1); // Make it alive

	DescriptorObjectTombstone out;
	out.m_block = block;
	out.m_idx = idx;

	return out;
}

} // end namespace anki
