// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/ShaderImpl.h>
#include <AnKi/Gr/Vulkan/GrManagerImpl.h>
#include <AnKi/Gr/Utils/Functions.h>
#include <SpirvCross/spirv_cross.hpp>

#define ANKI_DUMP_SHADERS 0

#if ANKI_DUMP_SHADERS
#	include <AnKi/Util/File.h>
#	include <AnKi/Gr/GrManager.h>
#endif

namespace anki {

class ShaderImpl::SpecConstsVector
{
public:
	spirv_cross::SmallVector<spirv_cross::SpecializationConstant> m_vec;
};

ShaderImpl::~ShaderImpl()
{
	for(auto& x : m_bindings)
	{
		x.destroy(getMemoryPool());
	}

	if(m_handle)
	{
		vkDestroyShaderModule(getDevice(), m_handle, nullptr);
	}

	if(m_specConstInfo.pMapEntries)
	{
		deleteArray(getMemoryPool(), const_cast<VkSpecializationMapEntry*>(m_specConstInfo.pMapEntries),
					m_specConstInfo.mapEntryCount);
	}

	if(m_specConstInfo.pData)
	{
		deleteArray(getMemoryPool(), static_cast<I32*>(const_cast<void*>(m_specConstInfo.pData)),
					m_specConstInfo.dataSize / sizeof(I32));
	}
}

Error ShaderImpl::init(const ShaderInitInfo& inf)
{
	ANKI_ASSERT(inf.m_binary.getSize() > 0);
	ANKI_ASSERT(m_handle == VK_NULL_HANDLE);
	m_shaderType = inf.m_shaderType;

#if ANKI_DUMP_SHADERS
	{
		StringRaii fnameSpirv(getAllocator());
		fnameSpirv.sprintf("%s/%s_t%u_%05u.spv", getManager().getCacheDirectory().cstr(), getName().cstr(),
						   U(m_shaderType), getUuid());

		File fileSpirv;
		ANKI_CHECK(fileSpirv.open(fnameSpirv.toCString(),
								  FileOpenFlag::kBinary | FileOpenFlag::kWrite | FileOpenFlag::kSpecial));
		ANKI_CHECK(fileSpirv.write(&inf.m_binary[0], inf.m_binary.getSize()));
	}
#endif

	VkShaderModuleCreateInfo ci = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, nullptr, 0, inf.m_binary.getSize(),
								   reinterpret_cast<const uint32_t*>(&inf.m_binary[0])};

	ANKI_VK_CHECK(vkCreateShaderModule(getDevice(), &ci, nullptr, &m_handle));

	// Get reflection info
	SpecConstsVector specConstIds;
	doReflection(inf.m_binary, specConstIds);

	// Set spec info
	if(specConstIds.m_vec.size())
	{
		const U32 constCount = U32(specConstIds.m_vec.size());

		m_specConstInfo.mapEntryCount = constCount;
		m_specConstInfo.pMapEntries = newArray<VkSpecializationMapEntry>(getMemoryPool(), constCount);
		m_specConstInfo.dataSize = constCount * sizeof(U32);
		m_specConstInfo.pData = newArray<U32>(getMemoryPool(), constCount);

		U32 count = 0;
		for(const spirv_cross::SpecializationConstant& sconst : specConstIds.m_vec)
		{
			// Set the entry
			VkSpecializationMapEntry& entry = const_cast<VkSpecializationMapEntry&>(m_specConstInfo.pMapEntries[count]);
			entry.constantID = sconst.constant_id;
			entry.offset = count * sizeof(U32);
			entry.size = sizeof(U32);

			// Find the value
			const ShaderSpecializationConstValue* val = nullptr;
			for(const ShaderSpecializationConstValue& v : inf.m_constValues)
			{
				if(v.m_constantId == entry.constantID)
				{
					val = &v;
					break;
				}
			}
			ANKI_ASSERT(val && "Contant ID wasn't found in the init info");

			// Copy the data
			U8* data = static_cast<U8*>(const_cast<void*>(m_specConstInfo.pData));
			data += entry.offset;
			*reinterpret_cast<U32*>(data) = val->m_uint;

			++count;
		}
	}

	return Error::kNone;
}

void ShaderImpl::doReflection(ConstWeakArray<U8> spirv, SpecConstsVector& specConstIds)
{
	spirv_cross::Compiler spvc(reinterpret_cast<const uint32_t*>(&spirv[0]), spirv.getSize() / sizeof(unsigned int));
	spirv_cross::ShaderResources rsrc = spvc.get_shader_resources();
	spirv_cross::ShaderResources rsrcActive = spvc.get_shader_resources(spvc.get_active_interface_variables());

	Array<U32, kMaxDescriptorSets> counts = {};
	Array2d<DescriptorBinding, kMaxDescriptorSets, kMaxBindingsPerDescriptorSet> descriptors;

	auto func = [&](const spirv_cross::SmallVector<spirv_cross::Resource>& resources, DescriptorType type) -> void {
		for(const spirv_cross::Resource& r : resources)
		{
			const U32 id = r.id;
			const U32 set = spvc.get_decoration(id, spv::Decoration::DecorationDescriptorSet);
			ANKI_ASSERT(set < kMaxDescriptorSets);
			const U32 binding = spvc.get_decoration(id, spv::Decoration::DecorationBinding);
			ANKI_ASSERT(binding < kMaxBindingsPerDescriptorSet);

			const spirv_cross::SPIRType& typeInfo = spvc.get_type(r.type_id);
			U32 arraySize = 1;
			if(typeInfo.array.size() != 0)
			{
				ANKI_ASSERT(typeInfo.array.size() == 1 && "Only 1D arrays are supported");
				arraySize = typeInfo.array[0];
				ANKI_ASSERT(arraySize > 0);
			}

			m_descriptorSetMask.set(set);
			m_activeBindingMask[set].set(set);

			// Images are special, they might be texel buffers
			if(type == DescriptorType::kTexture)
			{
				if(typeInfo.image.dim == spv::DimBuffer && typeInfo.image.sampled == 1)
				{
					type = DescriptorType::kReadTextureBuffer;
				}
				else if(typeInfo.image.dim == spv::DimBuffer && typeInfo.image.sampled == 2)
				{
					type = DescriptorType::kReadWriteTextureBuffer;
				}
			}

			// Check that there are no other descriptors with the same binding
			U32 foundIdx = kMaxU32;
			for(U32 i = 0; i < counts[set]; ++i)
			{
				if(descriptors[set][i].m_binding == binding)
				{
					foundIdx = i;
					break;
				}
			}

			if(foundIdx == kMaxU32)
			{
				// New binding, init it
				DescriptorBinding& descriptor = descriptors[set][counts[set]++];
				descriptor.m_binding = U8(binding);
				descriptor.m_type = type;
				descriptor.m_stageMask = ShaderTypeBit(1 << m_shaderType);
				descriptor.m_arraySize = arraySize;
			}
			else
			{
				// Same binding, make sure the type is compatible
				ANKI_ASSERT(type == descriptors[set][foundIdx].m_type && "Same binding different type");
				ANKI_ASSERT(arraySize == descriptors[set][foundIdx].m_arraySize && "Same binding different array size");
			}
		}
	};

	func(rsrc.uniform_buffers, DescriptorType::kUniformBuffer);
	func(rsrc.sampled_images, DescriptorType::kCombinedTextureSampler);
	func(rsrc.separate_images, DescriptorType::kTexture); // This also handles texture buffers
	func(rsrc.separate_samplers, DescriptorType::kSampler);
	func(rsrc.storage_buffers, DescriptorType::kStorageBuffer);
	func(rsrc.storage_images, DescriptorType::kImage);
	func(rsrc.acceleration_structures, DescriptorType::kAccelerationStructure);

	for(U32 set = 0; set < kMaxDescriptorSets; ++set)
	{
		if(counts[set])
		{
			m_bindings[set].create(getMemoryPool(), counts[set]);
			memcpy(&m_bindings[set][0], &descriptors[set][0], counts[set] * sizeof(DescriptorBinding));
		}
	}

	// Color attachments
	if(m_shaderType == ShaderType::kFragment)
	{
		for(const spirv_cross::Resource& r : rsrc.stage_outputs)
		{
			const U32 id = r.id;
			const U32 location = spvc.get_decoration(id, spv::Decoration::DecorationLocation);

			m_colorAttachmentWritemask.set(location);
		}
	}

	// Attribs
	if(m_shaderType == ShaderType::kVertex)
	{
		for(const spirv_cross::Resource& r : rsrcActive.stage_inputs)
		{
			const U32 id = r.id;
			const U32 location = spvc.get_decoration(id, spv::Decoration::DecorationLocation);

			m_attributeMask.set(location);
		}
	}

	// Spec consts
	specConstIds.m_vec = spvc.get_specialization_constants();

	// Push consts
	if(rsrc.push_constant_buffers.size() == 1)
	{
		const U32 blockSize =
			U32(spvc.get_declared_struct_size(spvc.get_type(rsrc.push_constant_buffers[0].base_type_id)));
		ANKI_ASSERT(blockSize > 0);
		ANKI_ASSERT(blockSize % 16 == 0 && "Should be aligned");
		ANKI_ASSERT(blockSize <= getGrManagerImpl().getDeviceCapabilities().m_pushConstantsSize);
		m_pushConstantsSize = blockSize;
	}
}

} // end namespace anki
