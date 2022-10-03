// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Vulkan/Common.h>
#include <AnKi/Gr/Vulkan/BufferImpl.h>
#include <AnKi/Gr/Vulkan/TextureImpl.h>
#include <AnKi/Gr/Vulkan/TextureViewImpl.h>
#include <AnKi/Gr/Vulkan/SamplerImpl.h>
#include <AnKi/Gr/Vulkan/AccelerationStructureImpl.h>
#include <AnKi/Util/WeakArray.h>
#include <AnKi/Util/BitSet.h>

namespace anki {

// Forward
class DSLayoutCacheEntry;

/// @addtogroup vulkan
/// @{

class alignas(8) DescriptorBinding
{
public:
	U32 m_arraySize = 0;
	ShaderTypeBit m_stageMask = ShaderTypeBit::kNone;
	DescriptorType m_type = DescriptorType::kCount;
	U8 m_binding = kMaxU8;
};
static_assert(sizeof(DescriptorBinding) == 8, "Should be packed because it will be hashed");

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
	VkImageView m_imgViewHandle;
	VkSampler m_samplerHandle;
	VkImageLayout m_layout;
};

class TextureBinding
{
public:
	VkImageView m_imgViewHandle;
	VkImageLayout m_layout;
};

class SamplerBinding
{
public:
	VkSampler m_samplerHandle;
};

class BufferBinding
{
public:
	VkBuffer m_buffHandle;
	PtrSize m_offset;
	PtrSize m_range;
};

class ImageBinding
{
public:
	VkImageView m_imgViewHandle;
};

class AsBinding
{
public:
	VkAccelerationStructureKHR m_accelerationStructureHandle;
};

class TextureBufferBinding
{
public:
	VkBufferView m_buffView;
};

class AnyBinding
{
public:
	Array<U64, 2> m_uuids;

	union
	{
		TextureSamplerBinding m_texAndSampler;
		TextureBinding m_tex;
		SamplerBinding m_sampler;
		BufferBinding m_buff;
		ImageBinding m_image;
		AsBinding m_accelerationStructure;
		TextureBufferBinding m_textureBuffer;
	};

	DescriptorType m_type;
};
static_assert(std::is_trivial<AnyBinding>::value, "Shouldn't have constructor for perf reasons");

class AnyBindingExtended
{
public:
	union
	{
		AnyBinding m_single;
		AnyBinding* m_array;
	};

	U32 m_arraySize;
};
static_assert(std::is_trivial<AnyBindingExtended>::value, "Shouldn't have constructor for perf reasons");

/// Descriptor set thin wraper.
class DescriptorSet
{
	friend class DescriptorSetFactory;
	friend class BindlessDescriptorSet;
	friend class DescriptorSetState;

public:
	VkDescriptorSet getHandle() const
	{
		ANKI_ASSERT(m_handle);
		return m_handle;
	}

private:
	VkDescriptorSet m_handle = VK_NULL_HANDLE;
};

/// A state tracker of descriptors.
class DescriptorSetState
{
	friend class DescriptorSetFactory;

public:
	void init(StackMemoryPool* pool)
	{
		m_pool = pool;
	}

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

	void bindTextureAndSampler(U32 binding, U32 arrayIdx, const TextureView* texView, const Sampler* sampler,
							   VkImageLayout layout)
	{
		const TextureViewImpl& viewImpl = static_cast<const TextureViewImpl&>(*texView);
		ANKI_ASSERT(viewImpl.getTextureImpl().isSubresourceGoodForSampling(viewImpl.getSubresource()));

		AnyBinding& b = getBindingToPopulate(binding, arrayIdx);
		b = {};
		b.m_type = DescriptorType::kCombinedTextureSampler;
		b.m_uuids[0] = viewImpl.getHash();
		b.m_uuids[1] = ptrToNumber(static_cast<const SamplerImpl*>(sampler)->m_sampler->getHandle());

		b.m_texAndSampler.m_imgViewHandle = viewImpl.getHandle();
		b.m_texAndSampler.m_samplerHandle = static_cast<const SamplerImpl*>(sampler)->m_sampler->getHandle();
		b.m_texAndSampler.m_layout = layout;

		m_dirtyBindings.set(binding);
		unbindBindlessDSet();
	}

	void bindTexture(U32 binding, U32 arrayIdx, const TextureView* texView, VkImageLayout layout)
	{
		const TextureViewImpl& viewImpl = static_cast<const TextureViewImpl&>(*texView);
		ANKI_ASSERT(viewImpl.getTextureImpl().isSubresourceGoodForSampling(viewImpl.getSubresource()));

		AnyBinding& b = getBindingToPopulate(binding, arrayIdx);
		b = {};
		b.m_type = DescriptorType::kTexture;
		b.m_uuids[0] = b.m_uuids[1] = viewImpl.getHash();

		b.m_tex.m_imgViewHandle = viewImpl.getHandle();
		b.m_tex.m_layout = layout;

		m_dirtyBindings.set(binding);
		unbindBindlessDSet();
	}

	void bindSampler(U32 binding, U32 arrayIdx, const Sampler* sampler)
	{
		AnyBinding& b = getBindingToPopulate(binding, arrayIdx);
		b = {};
		b.m_type = DescriptorType::kSampler;
		b.m_uuids[0] = b.m_uuids[1] = ptrToNumber(static_cast<const SamplerImpl*>(sampler)->m_sampler->getHandle());
		b.m_sampler.m_samplerHandle = static_cast<const SamplerImpl*>(sampler)->m_sampler->getHandle();

		m_dirtyBindings.set(binding);
		unbindBindlessDSet();
	}

	void bindUniformBuffer(U32 binding, U32 arrayIdx, const Buffer* buff, PtrSize offset, PtrSize range)
	{
		AnyBinding& b = getBindingToPopulate(binding, arrayIdx);
		b = {};
		b.m_type = DescriptorType::kUniformBuffer;
		b.m_uuids[0] = b.m_uuids[1] = buff->getUuid();

		b.m_buff.m_buffHandle = static_cast<const BufferImpl*>(buff)->getHandle();
		b.m_buff.m_offset = offset;
		b.m_buff.m_range = range;

		m_dirtyBindings.set(binding);
		unbindBindlessDSet();
	}

	void bindStorageBuffer(U32 binding, U32 arrayIdx, const Buffer* buff, PtrSize offset, PtrSize range)
	{
		AnyBinding& b = getBindingToPopulate(binding, arrayIdx);
		b = {};
		b.m_type = DescriptorType::kStorageBuffer;
		b.m_uuids[0] = b.m_uuids[1] = buff->getUuid();

		b.m_buff.m_buffHandle = static_cast<const BufferImpl*>(buff)->getHandle();
		b.m_buff.m_offset = offset;
		b.m_buff.m_range = range;

		m_dirtyBindings.set(binding);
		unbindBindlessDSet();
	}

	void bindReadOnlyTextureBuffer(U32 binding, U32 arrayIdx, const Buffer* buff, PtrSize offset, PtrSize range,
								   Format fmt)
	{
		const VkBufferView view = static_cast<const BufferImpl*>(buff)->getOrCreateBufferView(fmt, offset, range);
		AnyBinding& b = getBindingToPopulate(binding, arrayIdx);
		b = {};
		b.m_type = DescriptorType::kReadTextureBuffer;
		b.m_uuids[0] = ptrToNumber(view);
		b.m_uuids[1] = buff->getUuid();

		b.m_textureBuffer.m_buffView = view;

		m_dirtyBindings.set(binding);
		unbindBindlessDSet();
	}

	void bindImage(U32 binding, U32 arrayIdx, const TextureView* texView)
	{
		ANKI_ASSERT(texView);
		const TextureViewImpl* impl = static_cast<const TextureViewImpl*>(texView);
		ANKI_ASSERT(impl->getTextureImpl().isSubresourceGoodForImageLoadStore(impl->getSubresource()));

		AnyBinding& b = getBindingToPopulate(binding, arrayIdx);
		b = {};
		b.m_type = DescriptorType::kImage;
		ANKI_ASSERT(impl->getHash());
		b.m_uuids[0] = b.m_uuids[1] = impl->getHash();
		b.m_image.m_imgViewHandle = impl->getHandle();

		m_dirtyBindings.set(binding);
		unbindBindlessDSet();
	}

	void bindAccelerationStructure(U32 binding, U32 arrayIdx, const AccelerationStructure* as)
	{
		AnyBinding& b = getBindingToPopulate(binding, arrayIdx);
		b = {};
		b.m_type = DescriptorType::kAccelerationStructure;
		b.m_uuids[0] = b.m_uuids[1] = as->getUuid();
		b.m_accelerationStructure.m_accelerationStructureHandle =
			static_cast<const AccelerationStructureImpl*>(as)->getHandle();

		m_dirtyBindings.set(binding);
		unbindBindlessDSet();
	}

	/// Forget all the rest of the bindings and bind the whole bindless descriptor set.
	void bindBindlessDescriptorSet()
	{
		m_bindlessDSetBound = true;
		m_bindlessDSetDirty = true;
	}

private:
	StackMemoryPool* m_pool = nullptr;
	DescriptorSetLayout m_layout;

	Array<AnyBindingExtended, kMaxBindingsPerDescriptorSet> m_bindings;

	U64 m_lastHash = 0;

	BitSet<kMaxBindingsPerDescriptorSet, U32> m_dirtyBindings = {true};
	BitSet<kMaxBindingsPerDescriptorSet, U32> m_bindingSet = {false};
	Bool m_layoutDirty = true;
	Bool m_bindlessDSetDirty = true;
	Bool m_bindlessDSetBound = false;

	/// Only DescriptorSetFactory should call this.
	/// @param hash If hash is zero then the DS doesn't need rebind.
	void flush(U64& hash, Array<PtrSize, kMaxBindingsPerDescriptorSet>& dynamicOffsets, U32& dynamicOffsetCount,
			   Bool& bindlessDSet);

	void unbindBindlessDSet()
	{
		m_bindlessDSetBound = false;
	}

	AnyBinding& getBindingToPopulate(U32 bindingIdx, U32 arrayIdx);
};

/// Creates new descriptor set layouts and descriptor sets.
class DescriptorSetFactory
{
	friend class DSLayoutCacheEntry;

public:
	DescriptorSetFactory() = default;
	~DescriptorSetFactory();

	Error init(HeapMemoryPool* pool, VkDevice dev, U32 bindlessTextureCount, U32 bindlessTextureBuffers);

	void destroy();

	/// @note It's thread-safe.
	Error newDescriptorSetLayout(const DescriptorSetLayoutInitInfo& init, DescriptorSetLayout& layout);

	/// @note It's thread-safe.
	Error newDescriptorSet(StackMemoryPool& tmpPool, DescriptorSetState& state, DescriptorSet& set, Bool& dirty,
						   Array<PtrSize, kMaxBindingsPerDescriptorSet>& dynamicOffsets, U32& dynamicOffsetCount);

	void endFrame()
	{
		++m_frameCount;
	}

	/// Bind a sampled image.
	/// @note It's thread-safe.
	U32 bindBindlessTexture(const VkImageView view, const VkImageLayout layout);

	/// Bind a uniform texel buffer.
	/// @note It's thread-safe.
	U32 bindBindlessUniformTexelBuffer(const VkBufferView view);

	/// @note It's thread-safe.
	void unbindBindlessTexture(U32 idx);

	/// @note It's thread-safe.
	void unbindBindlessUniformTexelBuffer(U32 idx);

private:
	class BindlessDescriptorSet;
	class DSAllocator;
	class ThreadLocal;

	static thread_local ThreadLocal* m_threadLocal;
	DynamicArray<ThreadLocal*> m_allThreadLocals;
	Mutex m_allThreadLocalsMtx;

	HeapMemoryPool* m_pool = nullptr;
	VkDevice m_dev = VK_NULL_HANDLE;
	U64 m_frameCount = 0;

	DynamicArray<DSLayoutCacheEntry*> m_caches;
	SpinLock m_cachesMtx; ///< Not a mutex because after a while there will be no reason to lock

	BindlessDescriptorSet* m_bindless = nullptr;
	U32 m_bindlessTextureCount = kMaxU32;
	U32 m_bindlessUniformTexelBufferCount = kMaxU32;
};
/// @}

} // end namespace anki
