// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/vulkan/Common.h>
#include <anki/gr/vulkan/BufferImpl.h>
#include <anki/gr/vulkan/TextureImpl.h>
#include <anki/gr/vulkan/TextureViewImpl.h>
#include <anki/gr/vulkan/SamplerImpl.h>
#include <anki/gr/vulkan/AccelerationStructureImpl.h>
#include <anki/util/WeakArray.h>
#include <anki/util/BitSet.h>

namespace anki
{

// Forward
class DSThreadAllocator;
class DSLayoutCacheEntry;

/// @addtogroup vulkan
/// @{

class alignas(8) DescriptorBinding
{
public:
	ShaderTypeBit m_stageMask = ShaderTypeBit::NONE;
	DescriptorType m_type = DescriptorType::COUNT;
	U8 m_binding = MAX_U8;
	U8 m_arraySizeMinusOne = 0;
	Array<U8, 3> m_padding = {};
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
	void init(StackAllocator<U8>& alloc)
	{
		m_alloc = alloc;
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
		b.m_type = DescriptorType::COMBINED_TEXTURE_SAMPLER;
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
		b.m_type = DescriptorType::TEXTURE;
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
		b.m_type = DescriptorType::SAMPLER;
		b.m_uuids[0] = b.m_uuids[1] = ptrToNumber(static_cast<const SamplerImpl*>(sampler)->m_sampler->getHandle());
		b.m_sampler.m_samplerHandle = static_cast<const SamplerImpl*>(sampler)->m_sampler->getHandle();

		m_dirtyBindings.set(binding);
		unbindBindlessDSet();
	}

	void bindUniformBuffer(U32 binding, U32 arrayIdx, const Buffer* buff, PtrSize offset, PtrSize range)
	{
		AnyBinding& b = getBindingToPopulate(binding, arrayIdx);
		b = {};
		b.m_type = DescriptorType::UNIFORM_BUFFER;
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
		b.m_type = DescriptorType::STORAGE_BUFFER;
		b.m_uuids[0] = b.m_uuids[1] = buff->getUuid();

		b.m_buff.m_buffHandle = static_cast<const BufferImpl*>(buff)->getHandle();
		b.m_buff.m_offset = offset;
		b.m_buff.m_range = range;

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
		b.m_type = DescriptorType::IMAGE;
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
		b.m_type = DescriptorType::ACCELERATION_STRUCTURE;
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
	StackAllocator<U8> m_alloc;
	DescriptorSetLayout m_layout;

	Array<AnyBindingExtended, MAX_BINDINGS_PER_DESCRIPTOR_SET> m_bindings;

	U64 m_lastHash = 0;

	BitSet<MAX_BINDINGS_PER_DESCRIPTOR_SET, U32> m_dirtyBindings = {true};
	BitSet<MAX_BINDINGS_PER_DESCRIPTOR_SET, U32> m_bindingSet = {false};
	Bool m_layoutDirty = true;
	Bool m_bindlessDSetDirty = true;
	Bool m_bindlessDSetBound = false;

	/// Only DescriptorSetFactory should call this.
	/// @param hash If hash is zero then the DS doesn't need rebind.
	void flush(U64& hash, Array<PtrSize, MAX_BINDINGS_PER_DESCRIPTOR_SET>& dynamicOffsets, U32& dynamicOffsetCount,
			   Bool& bindlessDSet);

	void unbindBindlessDSet()
	{
		m_bindlessDSetBound = false;
	}

	AnyBinding& getBindingToPopulate(U32 bindingIdx, U32 arrayIdx)
	{
		ANKI_ASSERT(bindingIdx < MAX_BINDINGS_PER_DESCRIPTOR_SET);

		AnyBindingExtended& extended = m_bindings[bindingIdx];
		AnyBinding* out;
		const Bool bindingIsSet = m_bindingSet.get(bindingIdx);
		m_bindingSet.set(bindingIdx);
		extended.m_arraySize = (!bindingIsSet) ? 0 : extended.m_arraySize;

		if(ANKI_LIKELY(arrayIdx == 0 && extended.m_arraySize <= 1))
		{
			// Array idx is zero, most common case
			out = &extended.m_single;
			extended.m_arraySize = 1;
		}
		else if(arrayIdx < extended.m_arraySize)
		{
			// It's (or was) an array and there enough space in thar array
			out = &extended.m_array[arrayIdx];
		}
		else
		{
			// Need to grow
			const U32 newSize = max(extended.m_arraySize * 2, arrayIdx + 1);
			AnyBinding* newArr = m_alloc.newArray<AnyBinding>(newSize);

			if(extended.m_arraySize == 1)
			{
				newArr[0] = extended.m_single;
			}
			else if(extended.m_arraySize > 1)
			{
				// Copy old to new.
				memcpy(newArr, extended.m_array, sizeof(AnyBinding) * extended.m_arraySize);
			}

			// Zero the rest
			memset(newArr + extended.m_arraySize, 0, sizeof(AnyBinding) * (newSize - extended.m_arraySize));
			extended.m_arraySize = newSize;
			extended.m_array = newArr;

			// Return
			out = &extended.m_array[arrayIdx];
		}

		ANKI_ASSERT(out);
		return *out;
	}
};

/// Creates new descriptor set layouts and descriptor sets.
class DescriptorSetFactory
{
	friend class DSLayoutCacheEntry;
	friend class DSThreadAllocator;

public:
	DescriptorSetFactory() = default;
	~DescriptorSetFactory();

	ANKI_USE_RESULT Error init(const GrAllocator<U8>& alloc, VkDevice dev, const BindlessLimits& bindlessLimits);

	void destroy();

	/// @note It's thread-safe.
	ANKI_USE_RESULT Error newDescriptorSetLayout(const DescriptorSetLayoutInitInfo& init, DescriptorSetLayout& layout);

	/// @note It's thread-safe.
	ANKI_USE_RESULT Error newDescriptorSet(ThreadId tid, StackAllocator<U8>& tmpAlloc, DescriptorSetState& state,
										   DescriptorSet& set, Bool& dirty,
										   Array<PtrSize, MAX_BINDINGS_PER_DESCRIPTOR_SET>& dynamicOffsets,
										   U32& dynamicOffsetCount);

	void endFrame()
	{
		++m_frameCount;
	}

	/// Bind a sampled image.
	/// @note It's thread-safe.
	U32 bindBindlessTexture(const VkImageView view, const VkImageLayout layout);

	/// Bind a storage image.
	/// @note It's thread-safe.
	U32 bindBindlessImage(const VkImageView view);

	/// @note It's thread-safe.
	void unbindBindlessTexture(U32 idx);

	/// @note It's thread-safe.
	void unbindBindlessImage(U32 idx);

private:
	class BindlessDescriptorSet;

	GrAllocator<U8> m_alloc;
	VkDevice m_dev = VK_NULL_HANDLE;
	U64 m_frameCount = 0;

	DynamicArray<DSLayoutCacheEntry*> m_caches;
	SpinLock m_cachesMtx; ///< Not a mutex because after a while there will be no reason to lock

	BindlessDescriptorSet* m_bindless = nullptr;
	BindlessLimits m_bindlessLimits;
};
/// @}

} // end namespace anki
