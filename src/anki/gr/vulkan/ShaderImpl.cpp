// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/ShaderImpl.h>
#include <anki/gr/vulkan/GrManagerImpl.h>
#include <anki/gr/common/Misc.h>
#include <SPIRV-Cross/spirv_cross.hpp>

#define ANKI_DUMP_SHADERS ANKI_EXTRA_CHECKS

#if ANKI_DUMP_SHADERS
#	include <anki/util/File.h>
#	include <anki/gr/GrManager.h>
#endif

namespace anki
{

class ShaderImpl::SpecConstsVector
{
public:
	std::vector<spirv_cross::SpecializationConstant> m_vec;
};

ShaderImpl::~ShaderImpl()
{
	for(auto& x : m_bindings)
	{
		x.destroy(getAllocator());
	}

	if(m_handle)
	{
		vkDestroyShaderModule(getDevice(), m_handle, nullptr);
	}

	if(m_specConstInfo.pMapEntries)
	{
		getAllocator().deleteArray(
			const_cast<VkSpecializationMapEntry*>(m_specConstInfo.pMapEntries), m_specConstInfo.mapEntryCount);
	}

	if(m_specConstInfo.pData)
	{
		getAllocator().deleteArray(
			static_cast<I32*>(const_cast<void*>(m_specConstInfo.pData)), m_specConstInfo.dataSize / sizeof(I32));
	}
}

Error ShaderImpl::init(const ShaderInitInfo& inf)
{
	ANKI_ASSERT(inf.m_binary.getSize() > 0);
	ANKI_ASSERT(m_handle == VK_NULL_HANDLE);
	m_shaderType = inf.m_shaderType;

#if ANKI_DUMP_SHADERS
	{
		StringAuto fnameSpirv(getAllocator());
		fnameSpirv.sprintf("%s/%05u.spv", getManager().getCacheDirectory().cstr(), getUuid());

		File fileSpirv;
		ANKI_CHECK(fileSpirv.open(fnameSpirv.toCString(), FileOpenFlag::BINARY | FileOpenFlag::WRITE));
		ANKI_CHECK(fileSpirv.write(&inf.m_binary[0], inf.m_binary.getSize()));
	}
#endif

	VkShaderModuleCreateInfo ci = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		nullptr,
		0,
		inf.m_binary.getSize(),
		reinterpret_cast<const uint32_t*>(&inf.m_binary[0])};

	ANKI_VK_CHECK(vkCreateShaderModule(getDevice(), &ci, nullptr, &m_handle));

	// Get reflection info
	SpecConstsVector specConstIds;
	doReflection(inf.m_binary, specConstIds);

	// Set spec info
	if(specConstIds.m_vec.size())
	{
		const U constCount = specConstIds.m_vec.size();

		m_specConstInfo.mapEntryCount = constCount;
		m_specConstInfo.pMapEntries = getAllocator().newArray<VkSpecializationMapEntry>(constCount);
		m_specConstInfo.dataSize = constCount * sizeof(I32);
		m_specConstInfo.pData = getAllocator().newArray<I32>(constCount);

		U count = 0;
		for(const spirv_cross::SpecializationConstant& sconst : specConstIds.m_vec)
		{
			// Set the entry
			VkSpecializationMapEntry& entry = const_cast<VkSpecializationMapEntry&>(m_specConstInfo.pMapEntries[count]);
			entry.constantID = sconst.constant_id;
			entry.offset = count * sizeof(I32);
			entry.size = sizeof(I32);

			// Copy the data
			U8* data = static_cast<U8*>(const_cast<void*>(m_specConstInfo.pData));
			data += entry.offset;
			*reinterpret_cast<I32*>(data) = inf.m_constValues[sconst.constant_id].m_int;

			++count;
		}
	}

	return Error::NONE;
}

void ShaderImpl::doReflection(ConstWeakArray<U8> spirv, SpecConstsVector& specConstIds)
{
	spirv_cross::Compiler spvc(reinterpret_cast<const uint32_t*>(&spirv[0]), spirv.getSize() / sizeof(unsigned int));
	spirv_cross::ShaderResources rsrc = spvc.get_shader_resources();
	spirv_cross::ShaderResources rsrcActive = spvc.get_shader_resources(spvc.get_active_interface_variables());

	Array<U, MAX_DESCRIPTOR_SETS> counts = {{
		0,
	}};
	Array2d<DescriptorBinding, MAX_DESCRIPTOR_SETS, MAX_BINDINGS_PER_DESCRIPTOR_SET> descriptors;

	auto func = [&](const std::vector<spirv_cross::Resource>& resources,
					DescriptorType type,
					U minVkBinding,
					U maxVkBinding) -> void {
		for(const spirv_cross::Resource& r : resources)
		{
			const U32 id = r.id;
			const U32 set = spvc.get_decoration(id, spv::Decoration::DecorationDescriptorSet);
			const U32 binding = spvc.get_decoration(id, spv::Decoration::DecorationBinding);
			ANKI_ASSERT(binding >= minVkBinding && binding <= maxVkBinding);

			m_descriptorSetMask.set(set);
			m_activeBindingMask[set].set(set);

			// Check that there are no other descriptors with the same binding
			for(U i = 0; i < counts[set]; ++i)
			{
				ANKI_ASSERT(descriptors[set][i].m_binding != binding);
			}

			DescriptorBinding& descriptor = descriptors[set][counts[set]++];
			descriptor.m_binding = binding;
			descriptor.m_type = type;
			descriptor.m_stageMask = static_cast<ShaderTypeBit>(1 << m_shaderType);
		}
	};

	func(rsrc.uniform_buffers,
		DescriptorType::UNIFORM_BUFFER,
		MAX_TEXTURE_BINDINGS,
		MAX_TEXTURE_BINDINGS + MAX_UNIFORM_BUFFER_BINDINGS - 1);
	func(rsrc.sampled_images, DescriptorType::TEXTURE, 0, MAX_TEXTURE_BINDINGS - 1);
	func(rsrc.storage_buffers,
		DescriptorType::STORAGE_BUFFER,
		MAX_TEXTURE_BINDINGS + MAX_UNIFORM_BUFFER_BINDINGS,
		MAX_TEXTURE_BINDINGS + MAX_UNIFORM_BUFFER_BINDINGS + MAX_STORAGE_BUFFER_BINDINGS - 1);
	func(rsrc.storage_images,
		DescriptorType::IMAGE,
		MAX_TEXTURE_BINDINGS + MAX_UNIFORM_BUFFER_BINDINGS + MAX_STORAGE_BUFFER_BINDINGS,
		MAX_TEXTURE_BINDINGS + MAX_UNIFORM_BUFFER_BINDINGS + MAX_STORAGE_BUFFER_BINDINGS + MAX_IMAGE_BINDINGS - 1);

	for(U set = 0; set < MAX_DESCRIPTOR_SETS; ++set)
	{
		if(counts[set])
		{
			m_bindings[set].create(getAllocator(), counts[set]);
			memcpy(&m_bindings[set][0], &descriptors[set][0], counts[set] * sizeof(DescriptorBinding));
		}
	}

	// Color attachments
	if(m_shaderType == ShaderType::FRAGMENT)
	{
		for(const spirv_cross::Resource& r : rsrc.stage_outputs)
		{
			const U32 id = r.id;
			const U32 location = spvc.get_decoration(id, spv::Decoration::DecorationLocation);

			m_colorAttachmentWritemask.set(location);
		}
	}

	// Attribs
	if(m_shaderType == ShaderType::VERTEX)
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
		const U32 blockSize = spvc.get_declared_struct_size(spvc.get_type(rsrc.push_constant_buffers[0].base_type_id));
		ANKI_ASSERT(blockSize > 0);
		ANKI_ASSERT(blockSize % 16 == 0 && "Should be aligned");
		ANKI_ASSERT(blockSize <= getGrManagerImpl().getDeviceCapabilities().m_pushConstantsSize);
		m_pushConstantsSize = blockSize;
	}
}

} // end namespace anki
