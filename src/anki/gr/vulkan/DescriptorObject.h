// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/VulkanObject.h>
#include <anki/util/List.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

static const U DESCRIPTORS_PER_TOMBSTONE = 32;

class DescriptorTombstoneBlock : public IntrusiveListEnabled<DescriptorTombstoneBlock>
{
public:
	Array<Atomic<U8>, DESCRIPTORS_PER_TOMBSTONE> m_descriptors;
};

/// This object will tell you if the DescriptorObject is alive or not.
class DescriptorObjectTombstone
{
	friend class DescriptorObject;
	friend class DescriptorObjectTombstoneGenerator;

public:
	Bool isObjectAlive() const
	{
		return m_block->m_descriptors[m_idx].load() != 0;
	}

private:
	U8 m_idx;
	DescriptorTombstoneBlock* m_block;

	void killObject()
	{
		m_block->m_descriptors[m_idx].store(0);
	}
};

/// Contains special functionality for descriptor related types.
class DescriptorObject : public VulkanObject
{
public:
	DescriptorObject(GrManager* manager);

	~DescriptorObject();

	const DescriptorObjectTombstone& getDescriptorTombstone() const
	{
		return m_tombstone;
	}

private:
	DescriptorObjectTombstone m_tombstone;
};

class DescriptorObjectTombstoneGenerator
{
public:
	DescriptorObjectTombstoneGenerator() = default;

	~DescriptorObjectTombstoneGenerator();

	void init(GrAllocator<U8> alloc)
	{
		m_alloc = alloc;
	}

	/// @note It's thread-safe.
	DescriptorObjectTombstone newTombstone();

private:
	GrAllocator<U8> m_alloc;
	IntrusiveList<DescriptorTombstoneBlock> m_blocks;
	SpinLock m_blocksLock;
	U64 m_tombstoneCount = 0;
};
/// @}

} // end namespace anki
