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
		if(init.m_textures[i].m_texture)
		{
			++count;
			needsDSet = true;
			updateBindPoint(init.m_textures[i].m_usage);
			ANKI_ASSERT(
				init.m_textures[i].m_texture->getImplementation().usageValid(
					init.m_textures[i].m_usage));
		}

		if(init.m_textures[i].m_sampler)
		{
			++count;
			needsDSet = true;
		}
	}

	for(U i = 0; i < MAX_UNIFORM_BUFFER_BINDINGS; ++i)
	{
		if(init.m_uniformBuffers[i].m_buffer)
		{
			++count;
			needsDSet = true;
			updateBindPoint(init.m_uniformBuffers[i].m_usage);
			ANKI_ASSERT(!!(init.m_uniformBuffers[i].m_usage
							& BufferUsageBit::UNIFORM_ALL)
				&& !(init.m_uniformBuffers[i].m_usage
					   & ~BufferUsageBit::UNIFORM_ALL));
		}
		else if(init.m_uniformBuffers[i].m_uploadedMemory)
		{
			hasUploaded = true;
			needsDSet = true;
			updateBindPoint(init.m_uniformBuffers[i].m_usage);
			ANKI_ASSERT(!!(init.m_uniformBuffers[i].m_usage
							& BufferUsageBit::UNIFORM_ALL)
				&& !(init.m_uniformBuffers[i].m_usage
					   & ~BufferUsageBit::UNIFORM_ALL));
		}
	}

	for(U i = 0; i < MAX_STORAGE_BUFFER_BINDINGS; ++i)
	{
		if(init.m_storageBuffers[i].m_buffer)
		{
			++count;
			needsDSet = true;
			updateBindPoint(init.m_storageBuffers[i].m_usage);
			ANKI_ASSERT(!!(init.m_uniformBuffers[i].m_usage
							& BufferUsageBit::STORAGE_ALL)
				&& !(init.m_uniformBuffers[i].m_usage
					   & ~BufferUsageBit::STORAGE_ALL));
		}
		else if(init.m_storageBuffers[i].m_uploadedMemory)
		{
			hasUploaded = true;
			needsDSet = true;
			updateBindPoint(init.m_storageBuffers[i].m_usage);
			ANKI_ASSERT(!!(init.m_uniformBuffers[i].m_usage
							& BufferUsageBit::STORAGE_ALL)
				&& !(init.m_uniformBuffers[i].m_usage
					   & ~BufferUsageBit::STORAGE_ALL));
		}
	}

	for(U i = 0; i < MAX_IMAGE_BINDINGS; ++i)
	{
		if(init.m_images[i].m_texture)
		{
			++count;
			needsDSet = true;
			updateBindPoint(init.m_images[i].m_usage);
			ANKI_ASSERT(
				init.m_images[i].m_texture->getImplementation().usageValid(
					init.m_textures[i].m_usage));
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
	U texAndSamplerCount = 0;
	Array<VkDescriptorBufferInfo, MAX_UNIFORM_BUFFER_BINDINGS> unis = {{}};
	U uniCount = 0;
	Array<VkDescriptorBufferInfo, MAX_STORAGE_BUFFER_BINDINGS> storages = {{}};
	U storageCount = 0;
	Array<VkDescriptorImageInfo, MAX_IMAGE_BINDINGS> images = {{}};
	U imageCount = 0;
	Array<VkWriteDescriptorSet, 4> write = {{}};
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
				// No need to ref
			}

			inf.imageLayout =
				teximpl.computeLayout(init.m_textures[i].m_usage, 0);

			++texAndSamplerCount;
		}
	}

	if(texAndSamplerCount)
	{
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
					BufferUsageBit::UNIFORM_ALL);
			inf.range = VK_WHOLE_SIZE;

			m_dynamicBuffersMask.set(i);

			m_uniBindingCount = i + 1;
		}
	}

	if(uniCount)
	{
		VkWriteDescriptorSet& w = write[writeCount++];
		w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		w.descriptorCount = uniCount;
		w.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		w.dstBinding = MAX_TEXTURE_BINDINGS;
		w.pBufferInfo = &unis[0];
		w.dstSet = m_handle;
	}

	// 3nd the storage buffers
	for(U i = 0; i < MAX_STORAGE_BUFFER_BINDINGS; ++i)
	{
		if(init.m_storageBuffers[i].m_buffer)
		{
			VkDescriptorBufferInfo& inf = storages[storageCount++];
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

			m_storageBindingCount = i + 1;
		}
		else if(init.m_storageBuffers[i].m_uploadedMemory)
		{
			VkDescriptorBufferInfo& inf = storages[storageCount++];
			inf.buffer =
				getGrManagerImpl().getTransientMemoryManager().getBufferHandle(
					BufferUsageBit::STORAGE_ALL);
			inf.range = VK_WHOLE_SIZE;

			m_dynamicBuffersMask.set(MAX_UNIFORM_BUFFER_BINDINGS + i);

			m_storageBindingCount = i + 1;
		}
	}

	if(storageCount)
	{
		VkWriteDescriptorSet& w = write[writeCount++];
		w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		w.descriptorCount = storageCount;
		w.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
		w.dstBinding = MAX_TEXTURE_BINDINGS + MAX_UNIFORM_BUFFER_BINDINGS;
		w.pBufferInfo = &storages[0];
		w.dstSet = m_handle;
	}

	// 4th images
	for(U i = 0; i < MAX_IMAGE_BINDINGS; ++i)
	{
		const ImageBinding& binding = init.m_images[i];
		if(binding.m_texture)
		{
			VkDescriptorImageInfo& inf = images[imageCount++];
			const TextureImpl& tex = binding.m_texture->getImplementation();

			if(binding.m_level == 0)
			{
				inf.imageView = tex.m_viewHandle;
			}
			else
			{
				inf.imageView = tex.m_viewsEveryLevel[binding.m_level - 1];
			}
			inf.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

			m_refs[refCount++] = binding.m_texture;
		}
	}

	if(imageCount)
	{
		VkWriteDescriptorSet& w = write[writeCount++];
		w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		w.descriptorCount = imageCount;
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

} // end namespace anki
