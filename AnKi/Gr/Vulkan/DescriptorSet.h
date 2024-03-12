// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
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

class DSLayout
{
	friend class DSStateTracker;
	friend class DSLayoutFactory;
	friend class DSAllocator;

public:
	const VkDescriptorSetLayout& getHandle() const
	{
		ANKI_ASSERT(m_handle);
		return m_handle;
	}

private:
	VkDescriptorSetLayout m_handle = VK_NULL_HANDLE;

	U64 m_hash = 0;
	BitSet<kMaxBindingsPerDescriptorSet, U32> m_activeBindings = {false};
	Array<U32, kMaxBindingsPerDescriptorSet> m_bindingArraySize = {};
	Array<DescriptorType, kMaxBindingsPerDescriptorSet> m_bindingType = {};
	U32 m_minBinding = kMaxU32;
	U32 m_maxBinding = 0;
	U32 m_descriptorCount = 0;
};

/// Allocates descriptor sets.
class DSAllocator
{
	friend class DSStateTracker;

public:
	DSAllocator() = default;

	~DSAllocator();

	void destroy()
	{
		for(Block& b : m_blocks)
		{
			ANKI_ASSERT(b.m_pool != VK_NULL_HANDLE);
			vkDestroyDescriptorPool(getVkDevice(), b.m_pool, nullptr);
		}

		m_blocks.destroy();
		m_activeBlock = 0;
	}

	/// Reset for reuse.
	void reset();

private:
	class Block
	{
	public:
		VkDescriptorPool m_pool = VK_NULL_HANDLE;
		U32 m_dsetsAllocatedCount = 0;
	};

	static constexpr U32 kDescriptorSetGrowScale = 2;

	GrDynamicArray<Block> m_blocks;

	U32 m_activeBlock = 0;

	void allocate(const DSLayout& layout, VkDescriptorSet& set);

	void createNewBlock();
};

/// The bindless descriptor set.
class DSBindless : public MakeSingleton<DSBindless>
{
	friend class DSStateTracker;
	friend class DSLayoutFactory;

public:
	~DSBindless();

	Error init(U32 bindlessTextureCount, U32 bindlessTextureBuffers);

	/// Bind a sampled image.
	/// @note It's thread-safe.
	U32 bindTexture(const VkImageView view, const VkImageLayout layout);

	/// Bind a uniform texel buffer.
	/// @note It's thread-safe.
	U32 bindUniformTexelBuffer(VkBufferView view);

	/// @note It's thread-safe.
	void unbindTexture(U32 idx)
	{
		unbindCommon(idx, m_freeTexIndices, m_freeTexIndexCount);
	}

	/// @note It's thread-safe.
	void unbindUniformTexelBuffer(U32 idx)
	{
		unbindCommon(idx, m_freeTexelBufferIndices, m_freeTexelBufferIndexCount);
	}

	U32 getMaxTextureCount() const
	{
		return m_freeTexIndices.getSize();
	}

	U32 getMaxTexelBufferCount() const
	{
		return m_freeTexelBufferIndices.getSize();
	}

private:
	VkDescriptorSetLayout m_layout = VK_NULL_HANDLE;
	VkDescriptorPool m_dsPool = VK_NULL_HANDLE;
	VkDescriptorSet m_dset = VK_NULL_HANDLE;
	Mutex m_mtx;

	GrDynamicArray<U16> m_freeTexIndices;
	GrDynamicArray<U16> m_freeTexelBufferIndices;

	U16 m_freeTexIndexCount = kMaxU16;
	U16 m_freeTexelBufferIndexCount = kMaxU16;

	void unbindCommon(U32 idx, GrDynamicArray<U16>& freeIndices, U16& freeIndexCount);
};

/// A state tracker that creates descriptors at will.
class DSStateTracker
{
public:
	void init(StackMemoryPool* pool)
	{
		m_writeInfos = {pool};
	}

	Bool setLayout(const DSLayout* layout)
	{
		ANKI_ASSERT(layout);
		if(layout != m_layout)
		{
			m_layoutDirty = true;
			m_layout = layout;
			return true;
		}

		return false;
	}

	void setLayoutDirty()
	{
		m_layoutDirty = true;
	}

	void bindTexture(U32 binding, U32 arrayIdx, const TextureView* texView, VkImageLayout layout)
	{
		ANKI_ASSERT(texView);
		const TextureViewImpl& viewImpl = static_cast<const TextureViewImpl&>(*texView);
		ANKI_ASSERT(viewImpl.getTextureImpl().isSubresourceGoodForSampling(viewImpl.getSubresource()));
		Binding b;
		zeroMemory(b);
		b.m_type = DescriptorType::kTexture;
		b.m_image.imageView = viewImpl.getHandle();
		b.m_image.imageLayout = layout;
		b.m_image.sampler = VK_NULL_HANDLE;
		setBinding(binding, arrayIdx, b);
	}

	void bindSampler(U32 binding, U32 arrayIdx, const Sampler* sampler)
	{
		ANKI_ASSERT(sampler);
		Binding b;
		zeroMemory(b);
		b.m_type = DescriptorType::kSampler;
		b.m_image.sampler = static_cast<const SamplerImpl*>(sampler)->m_sampler->getHandle();
		setBinding(binding, arrayIdx, b);
	}

	void bindConstantBuffer(U32 binding, U32 arrayIdx, const Buffer* buff, PtrSize offset, PtrSize range)
	{
		ANKI_ASSERT(buff && range > 0);
		Binding b;
		zeroMemory(b);
		b.m_type = DescriptorType::kUniformBuffer;
		b.m_buffer.buffer = static_cast<const BufferImpl*>(buff)->getHandle();
		b.m_buffer.offset = offset;
		b.m_buffer.range = (range == kMaxPtrSize) ? VK_WHOLE_SIZE : range;
		setBinding(binding, arrayIdx, b);
	}

	void bindUavBuffer(U32 binding, U32 arrayIdx, const Buffer* buff, PtrSize offset, PtrSize range)
	{
		ANKI_ASSERT(buff && range > 0);
		Binding b;
		zeroMemory(b);
		b.m_type = DescriptorType::kStorageBuffer;
		b.m_buffer.buffer = static_cast<const BufferImpl*>(buff)->getHandle();
		b.m_buffer.offset = offset;
		b.m_buffer.range = (range == kMaxPtrSize) ? VK_WHOLE_SIZE : range;
		setBinding(binding, arrayIdx, b);
	}

	void bindReadOnlyTextureBuffer(U32 binding, U32 arrayIdx, const Buffer* buff, PtrSize offset, PtrSize range, Format fmt)
	{
		ANKI_ASSERT(buff && range > 0);
		Binding b;
		zeroMemory(b);
		b.m_type = DescriptorType::kReadTextureBuffer;
		b.m_bufferView = static_cast<const BufferImpl*>(buff)->getOrCreateBufferView(fmt, offset, range);
		setBinding(binding, arrayIdx, b);
	}

	void bindUavTexture(U32 binding, U32 arrayIdx, const TextureView* texView)
	{
		ANKI_ASSERT(texView);
		const TextureViewImpl* impl = static_cast<const TextureViewImpl*>(texView);
		ANKI_ASSERT(impl->getTextureImpl().isSubresourceGoodForImageLoadStore(impl->getSubresource()));
		Binding b;
		zeroMemory(b);
		b.m_type = DescriptorType::kImage;
		b.m_image.imageView = impl->getHandle();
		b.m_image.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		setBinding(binding, arrayIdx, b);
	}

	void bindAccelerationStructure(U32 binding, U32 arrayIdx, const AccelerationStructure* as)
	{
		ANKI_ASSERT(as);
		Binding b;
		zeroMemory(b);
		b.m_type = DescriptorType::kAccelerationStructure;
		b.m_as.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
		b.m_as.accelerationStructureCount = 1;
		b.m_as.pAccelerationStructures = &static_cast<const AccelerationStructureImpl*>(as)->getHandle();
		setBinding(binding, arrayIdx, b);
	}

	/// Forget all the rest of the bindings and bind the whole bindless descriptor set.
	void bindBindlessDescriptorSet()
	{
		if(!m_bindlessDSBound)
		{
			m_bindlessDSBound = true;
			m_bindlessDSDirty = true;
		}
	}

	/// Get all the state and create a descriptor set.
	/// @param[out] dsHandle It can be VK_NULL_HANDLE if there is no need to re-bind the descriptor set.
	Bool flush(DSAllocator& allocator, VkDescriptorSet& dsHandle);

private:
	class Binding
	{
	public:
		union
		{
			VkDescriptorImageInfo m_image;
			VkDescriptorBufferInfo m_buffer;
			VkWriteDescriptorSetAccelerationStructureKHR m_as;
			VkBufferView m_bufferView;
		};

		DescriptorType m_type;

		Binding()
		{
			// No init for perf reasons
		}
	};

	class BindingExtended
	{
	public:
		union
		{
			Binding m_single;
			Binding* m_array;
		};

		U32 m_count;

		BindingExtended()
		{
			// No init for perf reasons
		}
	};

	const DSLayout* m_layout = nullptr;

	Array<BindingExtended, kMaxBindingsPerDescriptorSet> m_bindings;

	BitSet<kMaxBindingsPerDescriptorSet, U32> m_bindingIsSetMask = {false};
	BitSet<kMaxBindingsPerDescriptorSet, U32> m_bindingDirtyMask = {true};
	Bool m_bindlessDSDirty = true;
	Bool m_bindlessDSBound = false;
	Bool m_layoutDirty = true;

	DynamicArray<VkWriteDescriptorSet, MemoryPoolPtrWrapper<StackMemoryPool>> m_writeInfos;

	void unbindBindlessDSet()
	{
		m_bindlessDSBound = false;
	}

	void setBinding(U32 bindingIdx, U32 arrayIdx, const Binding& b);
};

class alignas(8) DSBinding
{
public:
	U32 m_arraySize = 0;
	ShaderTypeBit m_stageMask = ShaderTypeBit::kNone;
	DescriptorType m_type = DescriptorType::kCount;
	U8 m_binding = kMaxU8;
};
static_assert(sizeof(DSBinding) == 8, "Should be packed because it will be hashed");

class DSLayoutFactory : public MakeSingleton<DSLayoutFactory>
{
public:
	DSLayoutFactory();

	~DSLayoutFactory();

	/// @note It's thread-safe.
	Error getOrCreateDescriptorSetLayout(const WeakArray<DSBinding>& bindings, const DSLayout*& layout);

private:
	GrDynamicArray<DSLayout*> m_layouts;
	Mutex m_mtx;
};

} // end namespace anki
