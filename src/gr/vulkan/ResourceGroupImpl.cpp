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
void ResourceGroupImpl::updateBindPoint(TextureUsageBit usage)
{
	ANKI_ASSERT(!!usage);

	VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_MAX_ENUM;
	const TextureUsageBit allCompute = TextureUsageBit::IMAGE_COMPUTE_READ_WRITE
		| TextureUsageBit::SAMPLED_COMPUTE;

	if(!!(usage & allCompute))
	{
		ANKI_ASSERT(
			!(usage & ~allCompute) && "Can't have compute with something else");
		bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
	}
	else
	{
		ANKI_ASSERT(!!(usage & ~allCompute));
		bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	}

	if(m_bindPoint != VK_PIPELINE_BIND_POINT_MAX_ENUM)
	{
		ANKI_ASSERT(m_bindPoint == bindPoint);
	}

	m_bindPoint = bindPoint;
}

//==============================================================================
void ResourceGroupImpl::updateBindPoint(BufferUsageBit usage)
{
	ANKI_ASSERT(!!usage);

	VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_MAX_ENUM;
	const BufferUsageBit allCompute = BufferUsageBit::UNIFORM_COMPUTE
		| BufferUsageBit::STORAGE_COMPUTE_READ_WRITE;

	if(!!(usage & allCompute))
	{
		ANKI_ASSERT(
			!(usage & ~allCompute) && "Can't have compute with something else");
		bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
	}
	else
	{
		ANKI_ASSERT(!!(usage & ~allCompute));
		bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	}

	if(m_bindPoint != VK_PIPELINE_BIND_POINT_MAX_ENUM)
	{
		ANKI_ASSERT(m_bindPoint == bindPoint);
	}

	m_bindPoint = bindPoint;
}

//==============================================================================
U ResourceGroupImpl::calcRefCount(
	const ResourceGroupInitInfo& init, Bool& hasUploaded, Bool& needsDSet)
{
	U count = 0;
	hasUploaded = false;
	needsDSet = false;

	for(U i = 0; i < MAX_TEXTURE_BINDINGS; ++i)
	{
		const TextureBinding& b = init.m_textures[i];
		if(b.m_texture)
		{
			++count;
			needsDSet = true;
			updateBindPoint(b.m_usage);
			ANKI_ASSERT(b.m_texture->getImplementation().usageValid(b.m_usage));
		}

		if(b.m_sampler)
		{
			++count;
			needsDSet = true;
		}
	}

	for(U i = 0; i < MAX_UNIFORM_BUFFER_BINDINGS; ++i)
	{
		const BufferBinding& b = init.m_uniformBuffers[i];
		if(b.m_buffer)
		{
			++count;
			needsDSet = true;
			updateBindPoint(b.m_usage);
			ANKI_ASSERT(!!(b.m_usage & BufferUsageBit::UNIFORM_ALL)
				&& !(b.m_usage & ~BufferUsageBit::UNIFORM_ALL));
			ANKI_ASSERT(b.m_buffer->getImplementation().usageValid(b.m_usage));
		}
		else if(b.m_uploadedMemory)
		{
			hasUploaded = true;
			needsDSet = true;
			updateBindPoint(b.m_usage);
			ANKI_ASSERT(!!(b.m_usage & BufferUsageBit::UNIFORM_ALL)
				&& !(b.m_usage & ~BufferUsageBit::UNIFORM_ALL));
		}
	}

	for(U i = 0; i < MAX_STORAGE_BUFFER_BINDINGS; ++i)
	{
		const BufferBinding& b = init.m_storageBuffers[i];
		if(b.m_buffer)
		{
			++count;
			needsDSet = true;
			updateBindPoint(b.m_usage);
			ANKI_ASSERT(!!(b.m_usage & BufferUsageBit::STORAGE_ALL)
				&& !(b.m_usage & ~BufferUsageBit::STORAGE_ALL));
			ANKI_ASSERT(b.m_buffer->getImplementation().usageValid(b.m_usage));
		}
		else if(b.m_uploadedMemory)
		{
			hasUploaded = true;
			needsDSet = true;
			updateBindPoint(b.m_usage);
			ANKI_ASSERT(!!(b.m_usage & BufferUsageBit::STORAGE_ALL)
				&& !(b.m_usage & ~BufferUsageBit::STORAGE_ALL));
		}
	}

	for(U i = 0; i < MAX_IMAGE_BINDINGS; ++i)
	{
		const ImageBinding& b = init.m_images[i];
		if(b.m_texture)
		{
			++count;
			needsDSet = true;
			updateBindPoint(b.m_usage);
			ANKI_ASSERT(b.m_texture->getImplementation().usageValid(b.m_usage));
		}
	}

	for(U i = 0; i < MAX_VERTEX_ATTRIBUTES; ++i)
	{
		const BufferBinding& b = init.m_vertexBuffers[i];
		if(b.m_buffer)
		{
			++count;
			ANKI_ASSERT(b.m_buffer->getImplementation().usageValid(
				BufferUsageBit::VERTEX));
		}
		else if(b.m_uploadedMemory)
		{
			hasUploaded = true;
		}
	}

	if(init.m_indexBuffer.m_buffer)
	{
		ANKI_ASSERT(init.m_indexBuffer.m_buffer->getImplementation().usageValid(
			BufferUsageBit::INDEX));
		++count;
	}

	return count;
}

//==============================================================================
Error ResourceGroupImpl::init(const ResourceGroupInitInfo& init)
{
	// Store the references
	//
	Bool hasUploaded = false;
	Bool needsDSet = false;
	U refCount = calcRefCount(init, hasUploaded, needsDSet);
	ANKI_ASSERT(refCount > 0 || hasUploaded);
	if(refCount)
	{
		m_refs.create(getAllocator(), refCount);
	}

	// Create DSet
	//
	if(needsDSet)
	{
		ANKI_CHECK(getGrManagerImpl().allocateDescriptorSet(m_handle));
		ANKI_ASSERT(m_bindPoint != VK_PIPELINE_BIND_POINT_MAX_ENUM);
	}

	// Update
	//
	Array<VkDescriptorImageInfo, MAX_TEXTURE_BINDINGS> texes = {{}};
	Array<VkDescriptorBufferInfo, MAX_UNIFORM_BUFFER_BINDINGS> unis = {{}};
	Array<VkDescriptorBufferInfo, MAX_STORAGE_BUFFER_BINDINGS> storages = {{}};
	Array<VkDescriptorImageInfo, MAX_IMAGE_BINDINGS> images = {{}};
	Array<VkWriteDescriptorSet, 4> write = {{}};
	U writeCount = 0;
	refCount = 0;
	Bool hole = false;
	U count = 0;

	// 1st the textures
	for(U i = 0; i < MAX_TEXTURE_BINDINGS; ++i)
	{
		if(init.m_textures[i].m_texture)
		{
			ANKI_ASSERT(!hole);
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
				// No need to ref
			}

			inf.imageLayout =
				teximpl.computeLayout(init.m_textures[i].m_usage, 0);

			++count;
		}
		else
		{
			hole = true;
		}
	}

	if(count)
	{
		VkWriteDescriptorSet& w = write[writeCount++];
		w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		w.descriptorCount = count;
		w.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		w.dstBinding = 0;
		w.pImageInfo = &texes[0];
		w.dstSet = m_handle;
	}

	// 2nd the uniform buffers
	count = 0;
	hole = false;
	for(U i = 0; i < MAX_UNIFORM_BUFFER_BINDINGS; ++i)
	{
		if(init.m_uniformBuffers[i].m_buffer)
		{
			ANKI_ASSERT(!hole);
			VkDescriptorBufferInfo& inf = unis[i];
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

			++count;
		}
		else if(init.m_uniformBuffers[i].m_uploadedMemory)
		{
			ANKI_ASSERT(!hole);
			VkDescriptorBufferInfo& inf = unis[i];
			inf.buffer =
				getGrManagerImpl().getTransientMemoryManager().getBufferHandle(
					BufferUsageBit::UNIFORM_ALL);
			inf.range = VK_WHOLE_SIZE;

			m_dynamicBuffersMask.set(i);

			++count;
		}
		else
		{
			hole = true;
		}
	}

	if(count)
	{
		VkWriteDescriptorSet& w = write[writeCount++];
		w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		w.descriptorCount = count;
		w.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		w.dstBinding = MAX_TEXTURE_BINDINGS;
		w.pBufferInfo = &unis[0];
		w.dstSet = m_handle;
	}

	m_uniBindingCount = count;

	// 3nd the storage buffers
	count = 0;
	hole = false;
	for(U i = 0; i < MAX_STORAGE_BUFFER_BINDINGS; ++i)
	{
		if(init.m_storageBuffers[i].m_buffer)
		{
			ANKI_ASSERT(!hole);
			VkDescriptorBufferInfo& inf = storages[i];
			inf.buffer = init.m_storageBuffers[i]
							 .m_buffer->getImplementation()
							 .getHandle();
			inf.offset = init.m_storageBuffers[i].m_offset;
			inf.range = init.m_storageBuffers[i].m_range;
			if(inf.range == 0)
			{
				inf.range = VK_WHOLE_SIZE;
			}

			m_refs[refCount++] = init.m_storageBuffers[i].m_buffer;
			++count;
		}
		else if(init.m_storageBuffers[i].m_uploadedMemory)
		{
			ANKI_ASSERT(!hole);
			VkDescriptorBufferInfo& inf = storages[i];
			inf.buffer =
				getGrManagerImpl().getTransientMemoryManager().getBufferHandle(
					BufferUsageBit::STORAGE_ALL);
			inf.range = VK_WHOLE_SIZE;

			m_dynamicBuffersMask.set(MAX_UNIFORM_BUFFER_BINDINGS + i);
			++count;
		}
		else
		{
			hole = true;
		}
	}

	if(count)
	{
		VkWriteDescriptorSet& w = write[writeCount++];
		w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		w.descriptorCount = count;
		w.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
		w.dstBinding = MAX_TEXTURE_BINDINGS + MAX_UNIFORM_BUFFER_BINDINGS;
		w.pBufferInfo = &storages[0];
		w.dstSet = m_handle;
	}

	m_storageBindingCount = count;

	// 4th images
	count = 0;
	hole = false;
	for(U i = 0; i < MAX_IMAGE_BINDINGS; ++i)
	{
		const ImageBinding& binding = init.m_images[i];
		if(binding.m_texture)
		{
			ANKI_ASSERT(!hole);
			VkDescriptorImageInfo& inf = images[i];
			TextureImpl& tex = binding.m_texture->getImplementation();

			inf.imageView = tex.getOrCreateSingleLevelView(binding.m_level);
			inf.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

			m_refs[refCount++] = binding.m_texture;
			++count;
		}
		else
		{
			hole = true;
		}
	}

	if(count)
	{
		VkWriteDescriptorSet& w = write[writeCount++];
		w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		w.descriptorCount = count;
		w.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		w.dstBinding = MAX_TEXTURE_BINDINGS + MAX_UNIFORM_BUFFER_BINDINGS
			+ MAX_STORAGE_BUFFER_BINDINGS;
		w.pImageInfo = &images[0];
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
	count = 0;
	hole = false;
	for(U i = 0; i < init.m_vertexBuffers.getSize(); ++i)
	{
		if(init.m_vertexBuffers[i].m_buffer)
		{
			ANKI_ASSERT(!hole);
			m_vert.m_buffs[i] = init.m_vertexBuffers[i]
									.m_buffer->getImplementation()
									.getHandle();
			m_vert.m_offsets[i] = init.m_vertexBuffers[i].m_offset;
			ANKI_ASSERT(m_vert.m_offsets[i] != MAX_PTR_SIZE);

			++count;
			m_refs[refCount++] = init.m_vertexBuffers[i].m_buffer;
		}
		else if(init.m_vertexBuffers[i].m_uploadedMemory)
		{
			ANKI_ASSERT(!hole);
			m_vert.m_buffs[i] =
				getGrManagerImpl().getTransientMemoryManager().getBufferHandle(
					BufferUsageBit::VERTEX);

			// Don't care about the offset
			m_vert.m_offsets[i] = MAX_PTR_SIZE;

			++count;
		}
		else
		{
			hole = true;
		}
	}
	m_vert.m_bindingCount = count;

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
	else if(init.m_indexBuffer.m_uploadedMemory)
	{
		ANKI_ASSERT(0 && "TODO");
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
				ANKI_ASSERT(!!(token.m_usage & BufferUsageBit::UNIFORM_ALL));
				dynOffsets[i] = token.m_offset;
			}
		}

		for(U i = 0; i < m_storageBindingCount; ++i)
		{
			if(m_dynamicBuffersMask.get(MAX_UNIFORM_BUFFER_BINDINGS + i))
			{
				// Uploaded
				const TransientMemoryToken& token =
					dynInfo->m_storageBuffers[i];

				ANKI_ASSERT(token.m_range);
				ANKI_ASSERT(!!(token.m_usage & BufferUsageBit::STORAGE_ALL));
				dynOffsets[MAX_UNIFORM_BUFFER_BINDINGS + i] = token.m_offset;
			}
		}
	}
}

//==============================================================================
void ResourceGroupImpl::getVertexBindingInfo(const TransientMemoryInfo* trans,
	VkBuffer buffers[],
	VkDeviceSize offsets[],
	U& bindingCount) const
{
	ANKI_ASSERT(buffers && offsets);
	bindingCount = m_vert.m_bindingCount;

	for(U i = 0; i < m_vert.m_bindingCount; ++i)
	{
		buffers[i] = m_vert.m_buffs[i];

		if(m_vert.m_offsets[i] != MAX_PTR_SIZE)
		{
			offsets[i] = m_vert.m_offsets[i];
		}
		else
		{
			// Transient
			ANKI_ASSERT(trans);
			offsets[i] = trans->m_vertexBuffers[i].m_offset;
		}

		ANKI_ASSERT(buffers[i] != VK_NULL_HANDLE);
	}
}

} // end namespace anki
