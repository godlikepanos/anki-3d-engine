// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/ResourceGroupImpl.h>
#include <anki/gr/ResourceGroup.h>
#include <anki/gr/vulkan/GrManagerImpl.h>
#include <anki/gr/vulkan/BufferImpl.h>

namespace anki
{

//==============================================================================
ResourceGroupImpl::~ResourceGroupImpl()
{
	if(m_handle)
	{
		getGrManagerImpl().freeDescriptorSet(m_handle);
	}

	m_refs.destroy(getAllocator());
}

//==============================================================================
U ResourceGroupImpl::calcRefCount(
	const ResourceGroupInitInfo& init, Bool& hasUploaded)
{
	U count = 0;
	hasUploaded = false;

	for(U i = 0; i < MAX_UNIFORM_BUFFER_BINDINGS; ++i)
	{
		if(init.m_uniformBuffers[i].m_buffer)
		{
			++count;
		}
		else if(init.m_uniformBuffers[i].m_uploadedMemory)
		{
			hasUploaded = true;
		}
	}

	for(U i = 0; i < MAX_VERTEX_ATTRIBUTES; ++i)
	{
		if(init.m_vertexBuffers[i].m_buffer)
		{
			++count;
		}
		else if(init.m_vertexBuffers[i].m_uploadedMemory)
		{
			hasUploaded = true;
		}
	}

	// TODO: The rest of the resources

	return count;
}

//==============================================================================
Error ResourceGroupImpl::init(const ResourceGroupInitInfo& init)
{
	// Store the references
	//
	Bool hasUploaded = false;
	U refCount = calcRefCount(init, hasUploaded);
	ANKI_ASSERT(refCount > 0 || hasUploaded);
	if(refCount)
	{
		m_refs.create(getAllocator(), refCount);
	}

	// Update
	//
	Array<VkDescriptorBufferInfo, MAX_UNIFORM_BUFFER_BINDINGS> unis = {{}};
	U uniCount = 0;
	Array<VkWriteDescriptorSet, 1> write = {{}};
	U writeCount = 0;
	refCount = 0;

	// 1st the textures
	// TODO

	// 2nd the uniform buffers
	for(U i = 0; i < MAX_UNIFORM_BUFFER_BINDINGS; ++i)
	{
		if(init.m_uniformBuffers[i].m_buffer)
		{
			VkDescriptorBufferInfo& inf = unis[uniCount++];
			inf.buffer = init.m_uniformBuffers[i]
							 .m_buffer->getImplementation()
							 .getHandle();
			inf.offset = init.m_uniformBuffers[i].m_offset;
			inf.range = init.m_uniformBuffers[i].m_range;

			m_refs[refCount++] = init.m_uniformBuffers[i].m_buffer;
		}
		else if(init.m_uniformBuffers[i].m_uploadedMemory)
		{
			ANKI_ASSERT(0 && "TODO");
		}
	}

	if(uniCount)
	{
		if(m_handle == VK_NULL_HANDLE)
		{
			ANKI_CHECK(getGrManagerImpl().allocateDescriptorSet(m_handle));
		}

		VkWriteDescriptorSet& w = write[writeCount++];
		w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		w.descriptorCount = uniCount;
		w.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		w.dstBinding = MAX_TEXTURE_BINDINGS;
		w.pBufferInfo = &unis[0];
		w.dstSet = m_handle;
	}

	// Check if it was created. It's not created only if the rc group contains
	// only vertex info.
	if(m_handle)
	{
		vkUpdateDescriptorSets(getDevice(), writeCount, &write[0], 0, nullptr);
	}

	// Vertex stuff
	//
	for(U i = 0; i < init.m_vertexBuffers.getSize(); ++i)
	{
		if(init.m_vertexBuffers[i].m_buffer)
		{
			m_vertBuffs[i] = init.m_vertexBuffers[i]
								 .m_buffer->getImplementation()
								 .getHandle();
			m_offsets[i] = init.m_vertexBuffers[i].m_offset;
			++m_bindingCount;

			m_refs[refCount++] = init.m_vertexBuffers[i].m_buffer;
		}
		else if(init.m_vertexBuffers[i].m_uploadedMemory)
		{
			ANKI_ASSERT(0 && "TODO");
			++m_bindingCount;
		}
		else
		{
			break;
		}
	}

	return ErrorCode::NONE;
}

} // end namespace anki