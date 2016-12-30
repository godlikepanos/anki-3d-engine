// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/Common.h>
#include <anki/gr/Buffer.h>
#include <anki/util/BitSet.h>

namespace anki
{

/// @addtogroup vulkan
/// @{

class alignas(4) DescriptorBinding
{
public:
	U8 m_type = MAX_U8;
	ShaderTypeBit m_stageMask = ShaderTypeBit::NONE;
	U8 m_binding = MAX_U8;
	U8 _m_padding = 0;
};

static_assert(sizeof(DescriptorBinding) == 4, "See file");

class DescriptorSetLayoutInitInfo
{
public:
	WeakArray<DescriptorBinding> m_bindings;
};

class DescriptorSetLayout
{
	friend class DescriptorSetFactory;

public:
	VkDescriptorSetLayout getHandle() const
	{
		ANKI_ASSERT(m_handle);
		return m_handle;
	}

	Bool isCreated() const
	{
		return m_handle != VK_NULL_HANDLE;
	}

private:
	VkDescriptorSetLayout m_handle = VK_NULL_HANDLE;
	U32 m_cacheEntryIdx = MAX_U32;
};

/// A state tracker of descriptors.
class DescriptorSetState
{
public:
	void bindUniformBuffer(U binding, const BufferPtr& buff, PtrSize offset, PtrSize range)
	{
		U realBinding = binding + MAX_TEXTURE_BINDINGS;
		m_setBindings.set(realBinding);
		m_uniformBuffers[binding].m_buff = buff;
		m_uniformBuffers[binding].m_offset = offset;
		m_uniformBuffers[binding].m_range = range;
	}

	void bindStorageBuffer(U binding, const BufferPtr& buff, PtrSize offset, PtrSize range)
	{
		U realBinding = binding + MAX_TEXTURE_BINDINGS + MAX_UNIFORM_BUFFER_BINDINGS;
		m_setBindings.set(realBinding);
		m_storageBuffers[binding].m_buff = buff;
		m_storageBuffers[binding].m_offset = offset;
		m_storageBuffers[binding].m_range = range;
	}

private:
	class TextureBinding
	{
	public:
		TexturePtr m_tex;
		SamplerPtr m_sampler;
	};

	class BufferBinding
	{
	public:
		BufferPtr m_buff;
		PtrSize m_offset = MAX_PTR_SIZE;
		PtrSize m_range = 0;
	};

	class ImageBinding
	{
	public:
		TexturePtr m_tex;
		U16 m_level = 0;
	};

	DescriptorSetLayout m_layout;
	Array<BufferBinding, MAX_UNIFORM_BUFFER_BINDINGS> m_uniformBuffers;
	Array<BufferBinding, MAX_STORAGE_BUFFER_BINDINGS> m_storageBuffers;
	Array<TextureBinding, MAX_TEXTURE_BINDINGS> m_textures;
	Array<ImageBinding, MAX_IMAGE_BINDINGS> m_images;

	BitSet<MAX_BINDINGS_PER_DESCRIPTOR_SET, U8> m_setBindings = {false};
};

class DescriptorSet
{
public:
	VkDescriptorSet getHandle() const
	{
		ANKI_ASSERT(m_handle);
		return m_handle;
	}

private:
	VkDescriptorSet m_handle = VK_NULL_HANDLE;
};

/// Creates new descriptor set layouts and descriptor sets.
class DescriptorSetFactory
{
public:
	DescriptorSetFactory() = default;
	~DescriptorSetFactory();

	void init(const GrAllocator<U8>& alloc, VkDevice dev);

	void destroy()
	{
		ANKI_ASSERT(!"TODO");
	}

	void newDescriptorSetLayout(const DescriptorSetLayoutInitInfo& init, DescriptorSetLayout& layout);

	void newDescriptorSet(const DescriptorSetState& init, DescriptorSet& set);

	void collectGarbage();

private:
	class Cache;

	GrAllocator<U8> m_alloc;
	VkDevice m_dev = VK_NULL_HANDLE;
	DynamicArray<Cache*> m_caches;
	Mutex m_mtx;

	Cache* newCacheEntry(const DescriptorBinding* bindings, U bindingCount, U64 hash);
};
/// @}

} // end namespace anki
