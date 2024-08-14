// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Vulkan/VkCommon.h>
#include <AnKi/Util/HashMap.h>
#include <AnKi/Util/WeakArray.h>

namespace anki {

/// @addtogroup vulkan
/// @{

/// Allocates descriptor sets.
class DescriptorAllocator
{
public:
	DescriptorAllocator() = default;

	~DescriptorAllocator();

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

	void allocate(VkDescriptorSetLayout layout, VkDescriptorSet& set);

	/// Reset for reuse.
	void reset();

private:
	class Block
	{
	public:
		VkDescriptorPool m_pool = VK_NULL_HANDLE;
		U32 m_dsetsAllocatedCount = 0;
		U32 m_maxDsets = 0;
	};

	static constexpr U32 kDescriptorSetGrowScale = 2;

	GrDynamicArray<Block> m_blocks;

	U32 m_activeBlock = 0;

	void createNewBlock();
};

/// The bindless descriptor set.
class BindlessDescriptorSet : public MakeSingleton<BindlessDescriptorSet>
{
	friend class PipelineLayoutFactory2;
	friend class DescriptorState;

public:
	~BindlessDescriptorSet();

	Error init();

	/// Bind a sampled image.
	/// @note It's thread-safe.
	U32 bindTexture(const VkImageView view, const VkImageLayout layout);

	/// @note It's thread-safe.
	void unbindTexture(U32 idx);

	U32 getMaxTextureCount() const
	{
		return m_freeTexIndices.getSize();
	}

private:
	VkDescriptorSetLayout m_layout = VK_NULL_HANDLE;
	VkDescriptorPool m_dsPool = VK_NULL_HANDLE;
	VkDescriptorSet m_dset = VK_NULL_HANDLE;
	Mutex m_mtx;

	GrDynamicArray<U16> m_freeTexIndices;

	U16 m_freeTexIndexCount = kMaxU16;
};

/// Wrapper over VkPipelineLayout
class PipelineLayout2
{
	friend class PipelineLayoutFactory2;
	friend class DescriptorState;

public:
	VkPipelineLayout getHandle() const
	{
		ANKI_ASSERT(m_handle);
		return m_handle;
	}

	const ShaderReflectionDescriptorRelated& getReflection() const
	{
		return m_refl;
	}

private:
	VkPipelineLayout m_handle = VK_NULL_HANDLE;
	ShaderReflectionDescriptorRelated m_refl;
	Array<VkDescriptorSetLayout, kMaxDescriptorSets> m_dsetLayouts = {};
	Array<U32, kMaxDescriptorSets> m_descriptorCounts = {};
	U8 m_dsetCount = 0;
};

/// Creator of pipeline layouts.
class PipelineLayoutFactory2 : public MakeSingleton<PipelineLayoutFactory2>
{
public:
	PipelineLayoutFactory2() = default;

	~PipelineLayoutFactory2();

	/// @note It's thread-safe.
	Error getOrCreatePipelineLayout(const ShaderReflectionDescriptorRelated& refl, PipelineLayout2*& layout);

private:
	class DescriptorSetLayout
	{
	public:
		VkDescriptorSetLayout m_handle = {};
	};

	GrHashMap<U64, PipelineLayout2*> m_pplLayouts;
	GrHashMap<U64, DescriptorSetLayout*> m_dsLayouts;

	Mutex m_mtx;

	Error getOrCreateDescriptorSetLayout(ConstWeakArray<ShaderReflectionBinding> bindings, DescriptorSetLayout*& layout);
};

/// Part of the command buffer that deals with descriptors.
class DescriptorState
{
public:
	void init(StackMemoryPool* tempPool)
	{
		for(auto& set : m_sets)
		{
			set.init(tempPool);
		}
	}

	void setPipelineLayout(const PipelineLayout2* layout, VkPipelineBindPoint bindPoint)
	{
		ANKI_ASSERT(layout);
		if(layout != m_pipelineLayout || bindPoint != m_pipelineBindPoint)
		{
			m_pipelineLayout = layout;
			m_pipelineBindPoint = bindPoint;
			m_pushConstantsDirty = m_pushConstantsDirty || (m_pushConstSize != m_pipelineLayout->m_refl.m_pushConstantsSize);

			for(U32 iset = 0; iset < m_pipelineLayout->m_dsetCount; ++iset)
			{
				if(m_sets[iset].m_dsLayout != m_pipelineLayout->m_dsetLayouts[iset])
				{
					m_sets[iset].m_dirty = true;
					m_sets[iset].m_dsLayout = m_pipelineLayout->m_dsetLayouts[iset];
				}
			}
		}
	}

	void bindSampledTexture(U32 space, U32 registerBinding, VkImageView view, VkImageLayout layout)
	{
		ANKI_ASSERT(view != 0 && layout != VK_IMAGE_LAYOUT_UNDEFINED);
		Descriptor& desc = getDescriptor(HlslResourceType::kSrv, space, registerBinding);
		desc.m_image.imageView = view;
		desc.m_image.imageLayout = layout;
		desc.m_image.sampler = 0;
#if ANKI_ASSERTIONS_ENABLED
		desc.m_type = DescriptorType::kSrvTexture;
#endif
	}

	void bindStorageTexture(U32 space, U32 registerBinding, VkImageView view)
	{
		ANKI_ASSERT(view);
		Descriptor& desc = getDescriptor(HlslResourceType::kUav, space, registerBinding);
		desc.m_image.imageView = view;
		desc.m_image.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		desc.m_image.sampler = 0;
#if ANKI_ASSERTIONS_ENABLED
		desc.m_type = DescriptorType::kUavTexture;
#endif
	}

	void bindSampler(U32 space, U32 registerBinding, VkSampler sampler)
	{
		ANKI_ASSERT(sampler);
		Descriptor& desc = getDescriptor(HlslResourceType::kSampler, space, registerBinding);
		desc.m_image.imageView = 0;
		desc.m_image.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		desc.m_image.sampler = sampler;
#if ANKI_ASSERTIONS_ENABLED
		desc.m_type = DescriptorType::kSampler;
#endif
	}

	void bindUniformBuffer(U32 space, U32 registerBinding, VkBuffer buffer, PtrSize offset, PtrSize range)
	{
		ANKI_ASSERT(buffer && range > 0);
		Descriptor& desc = getDescriptor(HlslResourceType::kCbv, space, registerBinding);
		desc.m_buffer.buffer = buffer;
		desc.m_buffer.offset = offset;
		desc.m_buffer.range = range;
#if ANKI_ASSERTIONS_ENABLED
		desc.m_type = DescriptorType::kConstantBuffer;
#endif
	}

	void bindReadStorageBuffer(U32 space, U32 registerBinding, VkBuffer buffer, PtrSize offset, PtrSize range)
	{
		ANKI_ASSERT(buffer && range > 0);
		Descriptor& desc = getDescriptor(HlslResourceType::kSrv, space, registerBinding);
		desc.m_buffer.buffer = buffer;
		desc.m_buffer.offset = offset;
		desc.m_buffer.range = range;
#if ANKI_ASSERTIONS_ENABLED
		desc.m_type = DescriptorType::kSrvStructuredBuffer;
#endif
	}

	void bindReadWriteStorageBuffer(U32 space, U32 registerBinding, VkBuffer buffer, PtrSize offset, PtrSize range)
	{
		ANKI_ASSERT(buffer && range > 0);
		Descriptor& desc = getDescriptor(HlslResourceType::kUav, space, registerBinding);
		desc.m_buffer.buffer = buffer;
		desc.m_buffer.offset = offset;
		desc.m_buffer.range = range;
#if ANKI_ASSERTIONS_ENABLED
		desc.m_type = DescriptorType::kUavStructuredBuffer;
#endif
	}

	void bindReadTexelBuffer(U32 space, U32 registerBinding, VkBufferView bufferView)
	{
		ANKI_ASSERT(bufferView);
		Descriptor& desc = getDescriptor(HlslResourceType::kSrv, space, registerBinding);
		desc.m_bufferView = bufferView;
#if ANKI_ASSERTIONS_ENABLED
		desc.m_type = DescriptorType::kSrvTexelBuffer;
#endif
	}

	void bindReadWriteTexelBuffer(U32 space, U32 registerBinding, VkBufferView bufferView)
	{
		ANKI_ASSERT(bufferView);
		Descriptor& desc = getDescriptor(HlslResourceType::kUav, space, registerBinding);
		desc.m_bufferView = bufferView;
#if ANKI_ASSERTIONS_ENABLED
		desc.m_type = DescriptorType::kUavTexelBuffer;
#endif
	}

	void bindAccelerationStructure(U32 space, U32 registerBinding, const VkAccelerationStructureKHR* handle)
	{
		ANKI_ASSERT(handle);
		Descriptor& desc = getDescriptor(HlslResourceType::kSrv, space, registerBinding);
		desc.m_as.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
		desc.m_as.pNext = nullptr;
		desc.m_as.accelerationStructureCount = 1;
		desc.m_as.pAccelerationStructures = handle;
#if ANKI_ASSERTIONS_ENABLED
		desc.m_type = DescriptorType::kAccelerationStructure;
#endif
	}

	void setPushConstants(const void* data, U32 dataSize)
	{
		ANKI_ASSERT(data && dataSize && dataSize <= kMaxPushConstantSize);
		memcpy(m_pushConsts.getBegin(), data, dataSize);
		m_pushConstSize = dataSize;
		m_pushConstantsDirty = true;
	}

	void flush(VkCommandBuffer cmdb, DescriptorAllocator& dalloc);

private:
	class Descriptor
	{
	public:
		union
		{
			VkDescriptorImageInfo m_image;
			VkDescriptorBufferInfo m_buffer;
			VkWriteDescriptorSetAccelerationStructureKHR m_as;
			VkBufferView m_bufferView;
		};

#if ANKI_ASSERTIONS_ENABLED
		DescriptorType m_type = DescriptorType::kCount;
#endif
	};

	class DescriptorSet
	{
	public:
		Array<DynamicArray<Descriptor, MemoryPoolPtrWrapper<StackMemoryPool>>, U32(HlslResourceType::kCount)> m_descriptors;

		DynamicArray<VkWriteDescriptorSet, MemoryPoolPtrWrapper<StackMemoryPool>> m_writeInfos;

		VkDescriptorSetLayout m_dsLayout = VK_NULL_HANDLE;

		Bool m_dirty = true; ///< Needs rebuild and rebind

		void init(StackMemoryPool* pool)
		{
			for(auto& dynArr : m_descriptors)
			{
				dynArr = {pool};
			}

			m_writeInfos = {pool};
		}
	};

	const PipelineLayout2* m_pipelineLayout = nullptr;
	VkPipelineBindPoint m_pipelineBindPoint = VK_PIPELINE_BIND_POINT_MAX_ENUM;
	Array<DescriptorSet, kMaxDescriptorSets> m_sets;
	Array<VkDescriptorSet, kMaxDescriptorSets> m_vkDsets = {};

	Array<U8, kMaxPushConstantSize> m_pushConsts;
	U32 m_pushConstSize = 0;
	Bool m_pushConstantsDirty = true;

	Descriptor& getDescriptor(HlslResourceType svv, U32 space, U32 registerBinding)
	{
		if(registerBinding >= m_sets[space].m_descriptors[svv].getSize())
		{
			m_sets[space].m_descriptors[svv].resize(registerBinding + 1);
		}
		m_sets[space].m_dirty = true;
		return m_sets[space].m_descriptors[svv][registerBinding];
	}
};
/// @}

} // end namespace anki
