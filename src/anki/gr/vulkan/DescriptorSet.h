// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/Common.h>
#include <anki/gr/vulkan/BufferImpl.h>
#include <anki/gr/vulkan/TextureImpl.h>
#include <anki/gr/vulkan/TextureViewImpl.h>
#include <anki/gr/vulkan/SamplerImpl.h>
#include <anki/util/BitSet.h>

namespace anki
{

// Forward
class DSThreadAllocator;
class DSLayoutCacheEntry;

/// @addtogroup vulkan
/// @{

class alignas(4) DescriptorBinding
{
public:
	DescriptorType m_type = DescriptorType::COUNT;
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
	friend class DescriptorSetState;

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
	DSLayoutCacheEntry* m_entry = nullptr;
};

class TextureBinding
{
public:
	const TextureViewImpl* m_texView = nullptr;
	const MicroSampler* m_sampler = nullptr;
	VkImageLayout m_layout = VK_IMAGE_LAYOUT_MAX_ENUM;
};

class BufferBinding
{
public:
	const BufferImpl* m_buff = nullptr;
	PtrSize m_offset = MAX_PTR_SIZE;
	PtrSize m_range = 0;
};

class ImageBinding
{
public:
	const TextureViewImpl* m_texView = nullptr;
};

class AnyBinding
{
public:
	DescriptorType m_type = DescriptorType::COUNT;
	Array<U64, 2> m_uuids = {};

	TextureBinding m_tex;
	BufferBinding m_buff;
	ImageBinding m_image;
};

/// A state tracker of descriptors.
class DescriptorSetState
{
	friend class DescriptorSetFactory;

public:
	void setLayout(const DescriptorSetLayout& layout)
	{
		m_layout = layout;
		m_layoutDirty = true;
	}

	void bindTextureAndSampler(U binding, const TextureView* texView, const Sampler* sampler, VkImageLayout layout)
	{
		const TextureViewImpl& viewImpl = static_cast<const TextureViewImpl&>(*texView);
		ANKI_ASSERT(viewImpl.m_tex->isSubresourceGoodForSampling(viewImpl.m_subresource));

		AnyBinding& b = m_bindings[binding];
		b = {};
		b.m_type = DescriptorType::TEXTURE;
		b.m_uuids[0] = viewImpl.m_hash;
		b.m_uuids[1] = sampler->getUuid();

		b.m_tex.m_texView = &viewImpl;
		b.m_tex.m_sampler = static_cast<const SamplerImpl*>(sampler)->m_sampler.get();
		b.m_tex.m_layout = layout;

		m_anyBindingDirty = true;
	}

	void bindUniformBuffer(U binding, const Buffer* buff, PtrSize offset, PtrSize range)
	{
		AnyBinding& b = m_bindings[binding];
		b = {};
		b.m_type = DescriptorType::UNIFORM_BUFFER;
		b.m_uuids[0] = b.m_uuids[1] = buff->getUuid();

		b.m_buff.m_buff = static_cast<const BufferImpl*>(buff);
		b.m_buff.m_offset = offset;
		b.m_buff.m_range = range;

		m_anyBindingDirty = true;
		m_dynamicOffsetDirty.set(binding);
	}

	void bindStorageBuffer(U binding, const Buffer* buff, PtrSize offset, PtrSize range)
	{
		AnyBinding& b = m_bindings[binding];
		b = {};
		b.m_type = DescriptorType::STORAGE_BUFFER;
		b.m_uuids[0] = b.m_uuids[1] = buff->getUuid();

		b.m_buff.m_buff = static_cast<const BufferImpl*>(buff);
		b.m_buff.m_offset = offset;
		b.m_buff.m_range = range;

		m_anyBindingDirty = true;
		m_dynamicOffsetDirty.set(binding);
	}

	void bindImage(U binding, const TextureView* texView)
	{
		ANKI_ASSERT(texView);
		const TextureViewImpl* impl = static_cast<const TextureViewImpl*>(texView);
		ANKI_ASSERT(impl->m_tex->isSubresourceGoodForImageLoadStore(impl->m_subresource));

		AnyBinding& b = m_bindings[binding];
		b = {};
		b.m_type = DescriptorType::IMAGE;
		ANKI_ASSERT(impl->m_hash);
		b.m_uuids[0] = b.m_uuids[1] = impl->m_hash;
		b.m_image.m_texView = impl;

		m_anyBindingDirty = true;
	}

private:
	DescriptorSetLayout m_layout;

	Array<AnyBinding, MAX_BINDINGS_PER_DESCRIPTOR_SET> m_bindings;

	Bool8 m_anyBindingDirty = true;
	Bool8 m_layoutDirty = true;
	BitSet<MAX_BINDINGS_PER_DESCRIPTOR_SET> m_dynamicOffsetDirty = {true};
	U64 m_lastHash = 0;

	/// Only DescriptorSetFactory should call this.
	void flush(Bool& stateDirty,
		U64& hash,
		Array<U32, MAX_UNIFORM_BUFFER_BINDINGS + MAX_STORAGE_BUFFER_BINDINGS>& dynamicOffsets,
		U& dynamicOffsetCount);
};

/// Descriptor set thin wraper.
class DescriptorSet
{
	friend class DescriptorSetFactory;

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
	friend class DSLayoutCacheEntry;
	friend class DSThreadAllocator;

public:
	DescriptorSetFactory() = default;
	~DescriptorSetFactory();

	void init(const GrAllocator<U8>& alloc, VkDevice dev);

	void destroy();

	/// @note It's thread-safe.
	ANKI_USE_RESULT Error newDescriptorSetLayout(const DescriptorSetLayoutInitInfo& init, DescriptorSetLayout& layout);

	/// @note Obviously not thread-safe.
	ANKI_USE_RESULT Error newDescriptorSet(ThreadId tid,
		DescriptorSetState& state,
		DescriptorSet& set,
		Bool& dirty,
		Array<U32, MAX_UNIFORM_BUFFER_BINDINGS + MAX_STORAGE_BUFFER_BINDINGS>& dynamicOffsets,
		U& dynamicOffsetCount);

	void endFrame()
	{
		++m_frameCount;
	}

private:
	GrAllocator<U8> m_alloc;
	VkDevice m_dev = VK_NULL_HANDLE;
	U64 m_frameCount = 0;

	DynamicArray<DSLayoutCacheEntry*> m_caches;
	SpinLock m_cachesMtx; ///< Not a mutex because after a while there will be no reason to lock
};
/// @}

} // end namespace anki
