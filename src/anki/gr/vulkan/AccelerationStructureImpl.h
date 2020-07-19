// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/AccelerationStructure.h>
#include <anki/gr/vulkan/VulkanObject.h>
#include <anki/gr/vulkan/GpuMemoryManager.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

/// @memberof AccelerationStructureImpl
class ASBottomLevelInfo
{
public:
	IndexType m_indexType = IndexType::COUNT;
	Format m_positionsFormat = Format::NONE;
	U32 m_indexCount = 0;
	U32 m_vertexCount = 0;
};

/// @memberof AccelerationStructureImpl
class ASTopLevelInfo
{
public:
	U32 m_bottomLevelCount = 0;
};

/// AccelerationStructure implementation.
class AccelerationStructureImpl final : public AccelerationStructure,
										public VulkanObject<AccelerationStructure, AccelerationStructureImpl>
{
public:
	AccelerationStructureImpl(GrManager* manager, CString name)
		: AccelerationStructure(manager, name)
	{
	}

	~AccelerationStructureImpl();

	ANKI_USE_RESULT Error init(const AccelerationStructureInitInfo& inf);

	VkAccelerationStructureKHR getHandle() const
	{
		ANKI_ASSERT(m_handle);
		return m_handle;
	}

	const ASBottomLevelInfo& getBottomLevelInfo() const
	{
		ANKI_ASSERT(m_type == AccelerationStructureType::BOTTOM_LEVEL);
		return m_bottomLevelInfo;
	}

	const ASTopLevelInfo& getTopLevelInfo() const
	{
		ANKI_ASSERT(m_type == AccelerationStructureType::TOP_LEVEL);
		return m_topLevelInfo;
	}

private:
	VkAccelerationStructureKHR m_handle = VK_NULL_HANDLE;
	GpuMemoryHandle m_memHandle;

	union
	{
		ASBottomLevelInfo m_bottomLevelInfo;
		ASTopLevelInfo m_topLevelInfo;
	};
};
/// @}

} // end namespace anki
