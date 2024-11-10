// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/ShaderCompiler/Spirv.h>
#include <AnKi/ShaderCompiler/Dxc.h>
#include <SpirvCross/spirv_cross.hpp>

namespace anki {

Error doReflectionSpirv(ConstWeakArray<U8> spirv, ShaderType type, ShaderReflection& refl, ShaderCompilerString& errorStr)
{
	spirv_cross::Compiler spvc(reinterpret_cast<const U32*>(&spirv[0]), spirv.getSize() / sizeof(U32));
	spirv_cross::ShaderResources rsrc = spvc.get_shader_resources();
	spirv_cross::ShaderResources rsrcActive = spvc.get_shader_resources(spvc.get_active_interface_variables());

	auto func = [&](const spirv_cross::SmallVector<spirv_cross::Resource>& resources, const DescriptorType origType) -> Error {
		for(const spirv_cross::Resource& r : resources)
		{
			const U32 id = r.id;

			const U32 set = spvc.get_decoration(id, spv::Decoration::DecorationDescriptorSet);
			const U32 binding = spvc.get_decoration(id, spv::Decoration::DecorationBinding);
			if(set >= kMaxRegisterSpaces && set != kDxcVkBindlessRegisterSpace)
			{
				errorStr.sprintf("Exceeded set for: %s", r.name.c_str());
				return Error::kUserData;
			}

			const spirv_cross::SPIRType& typeInfo = spvc.get_type(r.type_id);
			U32 arraySize = 1;
			if(typeInfo.array.size() != 0)
			{
				if(typeInfo.array.size() != 1)
				{
					errorStr.sprintf("Only 1D arrays are supported: %s", r.name.c_str());
					return Error::kUserData;
				}

				if(set == kDxcVkBindlessRegisterSpace && typeInfo.array[0] != 0)
				{
					errorStr.sprintf("Only the bindless descriptor set can be an unbound array: %s", r.name.c_str());
					return Error::kUserData;
				}

				arraySize = typeInfo.array[0];
			}

			// Images are special, they might be texel buffers
			DescriptorType type = origType;
			if((type == DescriptorType::kSrvTexture || type == DescriptorType::kUavTexture) && typeInfo.image.dim == spv::DimBuffer)
			{
				if(typeInfo.image.sampled == 1)
				{
					type = DescriptorType::kSrvTexelBuffer;
				}
				else
				{
					ANKI_ASSERT(typeInfo.image.sampled == 2);
					type = DescriptorType::kUavTexelBuffer;
				}
			}

			if(set == kDxcVkBindlessRegisterSpace)
			{
				// Bindless dset

				if(arraySize != 0)
				{
					errorStr.sprintf("Unexpected unbound array for bindless: %s", r.name.c_str());
				}

				if(type != DescriptorType::kSrvTexture)
				{
					errorStr.sprintf("Unexpected bindless binding: %s", r.name.c_str());
					return Error::kUserData;
				}

				refl.m_descriptor.m_hasVkBindlessDescriptorSet = true;
			}
			else
			{
				// Regular binding

				// Use the binding to find out if it's a read or write storage buffer, there is no other way
				if(origType == DescriptorType::kSrvStructuredBuffer
				   && (binding < kDxcVkBindingShifts[set][HlslResourceType::kSrv]
					   || binding >= kDxcVkBindingShifts[set][HlslResourceType::kSrv] + 1000))
				{
					type = DescriptorType::kUavStructuredBuffer;
				}

				const HlslResourceType hlslResourceType = descriptorTypeToHlslResourceType(type);
				if(binding < kDxcVkBindingShifts[set][hlslResourceType] || binding >= kDxcVkBindingShifts[set][hlslResourceType] + 1000)
				{
					errorStr.sprintf("Unexpected binding: %s", r.name.c_str());
					return Error::kUserData;
				}

				ShaderReflectionBinding akBinding;
				akBinding.m_registerBindingPoint = binding - kDxcVkBindingShifts[set][hlslResourceType];
				akBinding.m_arraySize = U16(arraySize);
				akBinding.m_type = type;

				refl.m_descriptor.m_bindings[set][refl.m_descriptor.m_bindingCounts[set]] = akBinding;
				++refl.m_descriptor.m_bindingCounts[set];
			}
		}

		return Error::kNone;
	};

	Error err = func(rsrc.uniform_buffers, DescriptorType::kConstantBuffer);
	if(err)
	{
		return err;
	}

	err = func(rsrc.separate_images, DescriptorType::kSrvTexture); // This also handles texel buffers
	if(err)
	{
		return err;
	}

	err = func(rsrc.separate_samplers, DescriptorType::kSampler);
	if(err)
	{
		return err;
	}

	err = func(rsrc.storage_buffers, DescriptorType::kSrvStructuredBuffer);
	if(err)
	{
		return err;
	}

	err = func(rsrc.storage_images, DescriptorType::kUavTexture);
	if(err)
	{
		return err;
	}

	err = func(rsrc.acceleration_structures, DescriptorType::kAccelerationStructure);
	if(err)
	{
		return err;
	}

	for(U32 i = 0; i < kMaxRegisterSpaces; ++i)
	{
		std::sort(refl.m_descriptor.m_bindings[i].getBegin(), refl.m_descriptor.m_bindings[i].getBegin() + refl.m_descriptor.m_bindingCounts[i]);
	}

	// Color attachments
	if(type == ShaderType::kPixel)
	{
		for(const spirv_cross::Resource& r : rsrc.stage_outputs)
		{
			const U32 id = r.id;
			const U32 location = spvc.get_decoration(id, spv::Decoration::DecorationLocation);

			refl.m_pixel.m_colorRenderTargetWritemask.set(location);
		}
	}

	// Push consts
	if(rsrc.push_constant_buffers.size() == 1)
	{
		const U32 blockSize = U32(spvc.get_declared_struct_size(spvc.get_type(rsrc.push_constant_buffers[0].base_type_id)));
		if(blockSize == 0 || (blockSize % 16) != 0 || blockSize > kMaxFastConstantsSize)
		{
			errorStr.sprintf("Incorrect push constants size");
			return Error::kUserData;
		}

		refl.m_descriptor.m_fastConstantsSize = blockSize;
	}

	// Attribs
	if(type == ShaderType::kVertex)
	{
		for(const spirv_cross::Resource& r : rsrcActive.stage_inputs)
		{
			VertexAttributeSemantic a = VertexAttributeSemantic::kCount;
#define ANKI_ATTRIB_NAME(x) "in.var." #x
			if(r.name == ANKI_ATTRIB_NAME(POSITION))
			{
				a = VertexAttributeSemantic::kPosition;
			}
			else if(r.name == ANKI_ATTRIB_NAME(NORMAL))
			{
				a = VertexAttributeSemantic::kNormal;
			}
			else if(r.name == ANKI_ATTRIB_NAME(TEXCOORD0) || r.name == ANKI_ATTRIB_NAME(TEXCOORD))
			{
				a = VertexAttributeSemantic::kTexCoord;
			}
			else if(r.name == ANKI_ATTRIB_NAME(COLOR))
			{
				a = VertexAttributeSemantic::kColor;
			}
			else if(r.name == ANKI_ATTRIB_NAME(MISC0) || r.name == ANKI_ATTRIB_NAME(MISC))
			{
				a = VertexAttributeSemantic::kMisc0;
			}
			else if(r.name == ANKI_ATTRIB_NAME(MISC1))
			{
				a = VertexAttributeSemantic::kMisc1;
			}
			else if(r.name == ANKI_ATTRIB_NAME(MISC2))
			{
				a = VertexAttributeSemantic::kMisc2;
			}
			else if(r.name == ANKI_ATTRIB_NAME(MISC3))
			{
				a = VertexAttributeSemantic::kMisc3;
			}
			else
			{
				errorStr.sprintf("Unexpected attribute name: %s", r.name.c_str());
				return Error::kUserData;
			}
#undef ANKI_ATTRIB_NAME

			refl.m_vertex.m_vertexAttributeMask |= VertexAttributeSemanticBit(1 << a);

			const U32 id = r.id;
			const U32 location = spvc.get_decoration(id, spv::Decoration::DecorationLocation);
			if(location > kMaxU8)
			{
				errorStr.sprintf("Too high location value for attribute: %s", r.name.c_str());
				return Error::kUserData;
			}

			refl.m_vertex.m_vkVertexAttributeLocations[a] = U8(location);
		}
	}

	// Discards?
	if(type == ShaderType::kPixel)
	{
		visitSpirv(ConstWeakArray<U32>(reinterpret_cast<const U32*>(&spirv[0]), spirv.getSize() / sizeof(U32)),
				   [&](U32 cmd, [[maybe_unused]] ConstWeakArray<U32> instructions) {
					   if(cmd == spv::OpKill)
					   {
						   refl.m_pixel.m_discards = true;
					   }
				   });
	}

	return Error::kNone;
}

} // end namespace anki
