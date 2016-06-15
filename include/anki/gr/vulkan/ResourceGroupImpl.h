// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/VulkanObject.h>

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

	VkDescriptorSet getHandle() const
	{
		ANKI_ASSERT(m_handle);
		return m_handle;
	}

private:
	VkDescriptorSet m_handle = VK_NULL_HANDLE;

	/// Holds the references to the resources. Used to release the references
	/// gracefully
	DynamicArray<GrObjectPtr<GrObject>> m_refs;

	static U calcRefCount(const ResourceGroupInitInfo& init, Bool& hasUploaded);
};
/// @}

} // end namespace anki
