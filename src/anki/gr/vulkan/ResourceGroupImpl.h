// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/VulkanObject.h>
#include <anki/util/BitSet.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

/// Resource group implementation.
class ResourceGroupImpl : public VulkanObject
{
public:
	ResourceGroupImpl(GrManager* manager)
		: VulkanObject(manager)
	{
	}

	~ResourceGroupImpl();

	ANKI_USE_RESULT Error init(const ResourceGroupInitInfo& init);

	Bool hasDescriptorSet() const
	{
		return m_handle != VK_NULL_HANDLE;
	}

	VkDescriptorSet getHandle() const
	{
		ANKI_ASSERT(m_handle);
		return m_handle;
	}

	void getVertexBindingInfo(
		const TransientMemoryInfo* trans, VkBuffer buffers[], VkDeviceSize offsets[], U& bindingCount) const;

	void setupDynamicOffsets(const TransientMemoryInfo* dynInfo, U32 dynOffsets[]) const;

	/// Get index buffer info.
	/// @return false if there is no index buffer.
	Bool getIndexBufferInfo(VkBuffer& buff, VkDeviceSize& offset, VkIndexType& idxType) const
	{
		if(m_indexBuffHandle)
		{
			buff = m_indexBuffHandle;
			offset = m_indexBufferOffset;
			idxType = m_indexType;
			return true;
		}

		return false;
	}

	VkPipelineBindPoint getPipelineBindPoint() const
	{
		ANKI_ASSERT(m_bindPoint != VK_PIPELINE_BIND_POINT_MAX_ENUM);
		return m_bindPoint;
	}

private:
	VkDescriptorSet m_handle = VK_NULL_HANDLE;
	VkPipelineBindPoint m_bindPoint = VK_PIPELINE_BIND_POINT_MAX_ENUM;

	class
	{
	public:
		Array<VkBuffer, MAX_VERTEX_ATTRIBUTES> m_buffs = {{}};
		Array<VkDeviceSize, MAX_VERTEX_ATTRIBUTES> m_offsets = {{}};
		U32 m_bindingCount = 0;
	} m_vert;

	// For dynamic binding
	U8 m_uniBindingCount = 0;
	U8 m_storageBindingCount = 0;
	BitSet<MAX_UNIFORM_BUFFER_BINDINGS + MAX_STORAGE_BUFFER_BINDINGS> m_dynamicBuffersMask = {false};

	// Index info
	VkBuffer m_indexBuffHandle = VK_NULL_HANDLE;
	U32 m_indexBufferOffset = MAX_U32;
	VkIndexType m_indexType = VK_INDEX_TYPE_MAX_ENUM;

	/// Holds the references to the resources. Used to release the references gracefully.
	DynamicArray<GrObjectPtr<GrObject>> m_refs;

	U calcRefCount(const ResourceGroupInitInfo& init, Bool& hasUploaded, Bool& needsDSet);

	void updateBindPoint(TextureUsageBit usage);
	void updateBindPoint(BufferUsageBit usage);
};
/// @}

} // end namespace anki
