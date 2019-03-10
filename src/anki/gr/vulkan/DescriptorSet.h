// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/Common.h>
#include <anki/gr/vulkan/BufferImpl.h>
#include <anki/gr/vulkan/TextureImpl.h>
#include <anki/gr/vulkan/TextureViewImpl.h>
#include <anki/gr/vulkan/SamplerImpl.h>
#include <anki/util/WeakArray.h>
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
	U8 m_arraySize = 0;
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

	Bool operator==(const DescriptorSetLayout& b) const
	{
		return m_entry == b.m_entry;
	}

	Bool operator!=(const DescriptorSetLayout& b) const
	{
		return !operator==(b);
	}

private:
	VkDescriptorSetLayout m_handle = VK_NULL_HANDLE;
	DSLayoutCacheEntry* m_entry = nullptr;
};

class TextureSamplerBinding
{
public:
	const TextureViewImpl* m_texView;
	const MicroSampler* m_sampler;
	VkImageLayout m_layout;
};

class TextureBinding
{
public:
	const TextureViewImpl* m_texView;
	VkImageLayout m_layout;
};

class SamplerBinding
{
public:
	const MicroSampler* m_sampler;
};

class BufferBinding
{
public:
	const BufferImpl* m_buff;
	PtrSize m_offset;
	PtrSize m_range;
};

class ImageBinding
{
public:
	const TextureViewImpl* m_texView;
};

class AnyBinding
{
public:
	DescriptorType m_type = DescriptorType::COUNT;
	Array<U64, 2> m_uuids = {};

	union
	{
		TextureSamplerBinding m_texAndSampler;
		TextureBinding m_tex;
		SamplerBinding m_sampler;
		BufferBinding m_buff;
		ImageBinding m_image;
	};
};

/// A state tracker of descriptors.
class DescriptorSetState
{
	friend class DescriptorSetFactory;

public:
	void setLayout(const DescriptorSetLayout& layout)
	{
		if(layout.isCreated())
		{
			m_layoutDirty = m_layout != layout;
		}
		else
		{
			m_layoutDirty = true;
		}

		m_layout = layout;
	}

	void bindTextureAndSampler(U binding, const TextureView* texView, const Sampler* sampler, VkImageLayout layout)
	{
		const TextureViewImpl& viewImpl = static_cast<const TextureViewImpl&>(*texView);
		ANKI_ASSERT(viewImpl.m_tex->isSubresourceGoodForSampling(viewImpl.getSubresource()));

		AnyBinding& b = m_bindings[binding];
		b = {};
		b.m_type = DescriptorType::COMBINED_TEXTURE_SAMPLER;
		b.m_uuids[0] = viewImpl.m_hash;
		b.m_uuids[1] = ptrToNumber(static_cast<const SamplerImpl*>(sampler)->m_sampler->getHandle());

		b.m_texAndSampler.m_texView = &viewImpl;
		b.m_texAndSampler.m_sampler = static_cast<const SamplerImpl*>(sampler)->m_sampler.get();
		b.m_texAndSampler.m_layout = layout;

		m_anyBindingDirty = true;
	}

	void bindTexture(U binding, const TextureView* texView, VkImageLayout layout)
	{
		const TextureViewImpl& viewImpl = static_cast<const TextureViewImpl&>(*texView);
		ANKI_ASSERT(viewImpl.m_tex->isSubresourceGoodForSampling(viewImpl.getSubresource()));

		AnyBinding& b = m_bindings[binding];
		b = {};
		b.m_type = DescriptorType::TEXTURE;
		b.m_uuids[0] = b.m_uuids[1] = viewImpl.m_hash;

		b.m_tex.m_texView = &viewImpl;
		b.m_tex.m_layout = layout;

		m_anyBindingDirty = true;
	}

	void bindSampler(U binding, const Sampler* sampler)
	{
		AnyBinding& b = m_bindings[binding];
		b = {};
		b.m_type = DescriptorType::SAMPLER;
		b.m_uuids[0] = b.m_uuids[1] = ptrToNumber(static_cast<const SamplerImpl*>(sampler)->m_sampler->getHandle());
		b.m_sampler.m_sampler = static_cast<const SamplerImpl*>(sampler)->m_sampler.get();

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
		ANKI_ASSERT(impl->m_tex->isSubresourceGoodForImageLoadStore(impl->getSubresource()));

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

	Bool m_anyBindingDirty = true;
	Bool m_layoutDirty = true;
	BitSet<MAX_BINDINGS_PER_DESCRIPTOR_SET> m_dynamicOffsetDirty = {true};
	U64 m_lastHash = 0;

	/// Only DescriptorSetFactory should call this.
	void flush(Bool& stateDirty,
		U64& hash,
		Array<U32, MAX_BINDINGS_PER_DESCRIPTOR_SET>& dynamicOffsets,
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
		Array<U32, MAX_BINDINGS_PER_DESCRIPTOR_SET>& dynamicOffsets,
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
