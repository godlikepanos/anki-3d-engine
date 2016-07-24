// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/ResourceGroupImpl.h>
#include <anki/gr/ResourceGroup.h>
#include <anki/gr/vulkan/GrManagerImpl.h>
#include <anki/gr/vulkan/TextureImpl.h>
#include <anki/gr/vulkan/SamplerImpl.h>
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

	for(U i = 0; i < MAX_TEXTURE_BINDINGS; ++i)
	{
		if(init.m_textures[i].m_texture)
		{
			++count;
		}

		if(init.m_textures[i].m_sampler)
		{
			++count;
		}
	}

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

	if(init.m_indexBuffer.m_buffer)
	{
		++count;
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
	Array<VkDescriptorImageInfo, MAX_TEXTURE_BINDINGS> texes = {{}};
	U texAndSamplerCount = 0;
	Array<VkDescriptorBufferInfo, MAX_UNIFORM_BUFFER_BINDINGS> unis = {{}};
	U uniCount = 0;
	Array<VkWriteDescriptorSet, 2> write = {{}};
	U writeCount = 0;
	refCount = 0;

	// 1st the textures
	for(U i = 0; i < MAX_TEXTURE_BINDINGS; ++i)
	{
		if(init.m_textures[i].m_texture)
		{
			TextureImpl& teximpl =
				init.m_textures[i].m_texture->getImplementation();

			VkDescriptorImageInfo& inf = texes[i];
			inf.imageView = teximpl.m_viewHandle;

			m_refs[refCount++] = init.m_textures[i].m_texture;

			if(init.m_textures[i].m_sampler)
			{
				inf.sampler =
					init.m_textures[i].m_sampler->getImplementation().m_handle;

				m_refs[refCount++] = init.m_textures[i].m_sampler;
			}
			else
			{
				inf.sampler = teximpl.m_sampler->getImplementation().m_handle;
				// No need for ref
			}

			// TODO need another layout
			inf.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			++texAndSamplerCount;
		}
	}

	if(texAndSamplerCount)
	{
		if(m_handle == VK_NULL_HANDLE)
		{
			ANKI_CHECK(getGrManagerImpl().allocateDescriptorSet(m_handle));
		}

		VkWriteDescriptorSet& w = write[writeCount++];
		w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		w.descriptorCount = texAndSamplerCount;
		w.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		w.dstBinding = 0;
		w.pImageInfo = &texes[0];
		w.dstSet = m_handle;
	}

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
			if(inf.range == 0)
			{
				inf.range = VK_WHOLE_SIZE;
			}

			m_refs[refCount++] = init.m_uniformBuffers[i].m_buffer;

			m_uniBindingCount = i + 1;
		}
		else if(init.m_uniformBuffers[i].m_uploadedMemory)
		{
			VkDescriptorBufferInfo& inf = unis[uniCount++];
			inf.buffer =
				getGrManagerImpl().getTransientMemoryManager().getBufferHandle(
					BufferUsageBit::UNIFORM_ANY_SHADER);
			inf.range = VK_WHOLE_SIZE;

			m_dynamicBuffersMask.set(i);

			m_uniBindingCount = i + 1;
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

	// TODO Storage buffers

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

	if(init.m_indexBuffer.m_buffer)
	{
		m_indexBuffHandle =
			init.m_indexBuffer.m_buffer->getImplementation().getHandle();
		m_indexBufferOffset = init.m_indexBuffer.m_offset;

		if(init.m_indexSize == 2)
		{
			m_indexType = VK_INDEX_TYPE_UINT16;
		}
		else
		{
			ANKI_ASSERT(init.m_indexSize == 4);
			m_indexType = VK_INDEX_TYPE_UINT32;
		}

		m_refs[refCount++] = init.m_indexBuffer.m_buffer;
	}

	ANKI_ASSERT(refCount == m_refs.getSize());

	return ErrorCode::NONE;
}

//==============================================================================
void ResourceGroupImpl::setupDynamicOffsets(
	const TransientMemoryInfo* dynInfo, U32 dynOffsets[]) const
{
	if(m_dynamicBuffersMask.getAny())
	{
		// Has at least one uploaded buffer

		ANKI_ASSERT(
			dynInfo && "Need to provide dynInfo if there are uploaded buffers");

		for(U i = 0; i < m_uniBindingCount; ++i)
		{
			if(m_dynamicBuffersMask.get(i))
			{
				// Uploaded
				const TransientMemoryToken& token =
					dynInfo->m_uniformBuffers[i];

				ANKI_ASSERT(token.m_range);
				ANKI_ASSERT((token.m_usage & BufferUsageBit::UNIFORM_ANY_SHADER)
					!= BufferUsageBit::NONE);
				dynOffsets[i] = token.m_offset;
			}
		}
	}
}

} // end namespace anki
