// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/Common.h>
#include <anki/gr/Buffer.h>
#include <anki/gr/Texture.h>
#include <anki/gr/vulkan/TextureImpl.h>
#include <anki/gr/Sampler.h>
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
	TextureImpl* m_tex = nullptr;
	SamplerImpl* m_sampler = nullptr;
	DepthStencilAspectBit m_aspect = DepthStencilAspectBit::NONE;
	VkImageLayout m_layout = VK_IMAGE_LAYOUT_MAX_ENUM;
};

class BufferBinding
{
public:
	BufferImpl* m_buff = nullptr;
	PtrSize m_offset = MAX_PTR_SIZE;
	PtrSize m_range = 0;
};

class ImageBinding
{
public:
	TextureImpl* m_tex = nullptr;
	U16 m_level = 0;
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

	void bindTexture(U binding, Texture* tex, DepthStencilAspectBit aspect, VkImageLayout layout)
	{
		AnyBinding& b = m_bindings[binding];
		b = {};
		b.m_type = DescriptorType::TEXTURE;
		b.m_uuids[0] = b.m_uuids[1] = tex->getUuid();

		b.m_tex.m_tex = tex->m_impl.get();
		b.m_tex.m_sampler = tex->m_impl->m_sampler->m_impl.get();
		b.m_tex.m_aspect = aspect;
		b.m_tex.m_layout = layout;

		m_anyBindingDirty = true;
	}

	void bindTextureAndSampler(
		U binding, Texture* tex, Sampler* sampler, DepthStencilAspectBit aspect, VkImageLayout layout)
	{
		AnyBinding& b = m_bindings[binding];
		b = {};
		b.m_type = DescriptorType::TEXTURE;
		b.m_uuids[0] = tex->getUuid();
		b.m_uuids[1] = sampler->getUuid();

		b.m_tex.m_tex = tex->m_impl.get();
		b.m_tex.m_sampler = sampler->m_impl.get();
		b.m_tex.m_aspect = aspect;
		b.m_tex.m_layout = layout;

		m_anyBindingDirty = true;
	}

	void bindUniformBuffer(U binding, Buffer* buff, PtrSize offset, PtrSize range)
	{
		AnyBinding& b = m_bindings[binding];
		b = {};
		b.m_type = DescriptorType::UNIFORM_BUFFER;
		b.m_uuids[0] = b.m_uuids[1] = buff->getUuid();

		b.m_buff.m_buff = buff->m_impl.get();
		b.m_buff.m_offset = offset;
		b.m_buff.m_range = range;

		m_anyBindingDirty = true;
		m_dynamicOffsetDirty.set(binding);
	}

	void bindStorageBuffer(U binding, Buffer* buff, PtrSize offset, PtrSize range)
	{
		AnyBinding& b = m_bindings[binding];
		b = {};
		b.m_type = DescriptorType::STORAGE_BUFFER;
		b.m_uuids[0] = b.m_uuids[1] = buff->getUuid();

		b.m_buff.m_buff = buff->m_impl.get();
		b.m_buff.m_offset = offset;
		b.m_buff.m_range = range;

		m_anyBindingDirty = true;
		m_dynamicOffsetDirty.set(binding);
	}

	void bindImage(U binding, Texture* tex, U32 level)
	{
		AnyBinding& b = m_bindings[binding];
		b = {};
		b.m_type = DescriptorType::IMAGE;
		b.m_uuids[0] = b.m_uuids[1] = tex->getUuid();

		b.m_image.m_tex = tex->m_impl.get();
		b.m_image.m_level = level;

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
