// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Common.h>
#include <AnKi/Gr/GrObject.h>
#include <AnKi/Math.h>

namespace anki {

/// @warning Don't use Array because the compilers can't handle it for some reason.
inline constexpr ShaderVariableDataTypeInfo kShaderVariableDataTypeInfos[] = {
#define ANKI_SVDT_MACRO(type, baseType, rowCount, columnCount, isIntagralType) {ANKI_STRINGIZE(type), sizeof(type), false, isIntagralType},
#define ANKI_SVDT_MACRO_OPAQUE(constant, type) {ANKI_STRINGIZE(type), kMaxU32, true, false},
#include <AnKi/Gr/ShaderVariableDataType.def.h>
#undef ANKI_SVDT_MACRO
#undef ANKI_SVDT_MACRO_OPAQUE
};

void GrObjectDeleter::operator()(GrObject* ptr)
{
	deleteInstance(GrMemoryPool::getSingleton(), ptr);
}

const ShaderVariableDataTypeInfo& getShaderVariableDataTypeInfo(ShaderVariableDataType type)
{
	ANKI_ASSERT(type > ShaderVariableDataType::kNone && type < ShaderVariableDataType::kCount);
	return kShaderVariableDataTypeInfos[U32(type) - 1];
}

FormatInfo getFormatInfo(Format fmt)
{
	FormatInfo out = {};
	switch(fmt)
	{
#define ANKI_FORMAT_DEF(type, vk, d3d, componentCount, texelSize, blockWidth, blockHeight, blockSize, shaderType, depthStencil) \
	case Format::k##type: \
		out = {componentCount,      texelSize, blockWidth, blockHeight, blockSize, shaderType, DepthStencilAspectBit::k##depthStencil, \
			   ANKI_STRINGIZE(type)}; \
		break;
#include <AnKi/Gr/BackendCommon/Format.def.h>
#undef ANKI_FORMAT_DEF

	default:
		ANKI_ASSERT(0);
	}

	return out;
}

PtrSize computeSurfaceSize(U32 width32, U32 height32, Format fmt)
{
	const PtrSize width = width32;
	const PtrSize height = height32;
	ANKI_ASSERT(width > 0 && height > 0);
	ANKI_ASSERT(fmt != Format::kNone);

	const FormatInfo inf = getFormatInfo(fmt);

	if(inf.m_blockSize > 0)
	{
		// Compressed
		ANKI_ASSERT((width % inf.m_blockWidth) == 0);
		ANKI_ASSERT((height % inf.m_blockHeight) == 0);
		return (width / inf.m_blockWidth) * (height / inf.m_blockHeight) * inf.m_blockSize;
	}
	else
	{
		return width * height * inf.m_texelSize;
	}
}

PtrSize computeVolumeSize(U32 width32, U32 height32, U32 depth32, Format fmt)
{
	const PtrSize width = width32;
	const PtrSize height = height32;
	const PtrSize depth = depth32;
	ANKI_ASSERT(width > 0 && height > 0 && depth > 0);
	ANKI_ASSERT(fmt != Format::kNone);

	const FormatInfo inf = getFormatInfo(fmt);

	if(inf.m_blockSize > 0)
	{
		// Compressed
		ANKI_ASSERT((width % inf.m_blockWidth) == 0);
		ANKI_ASSERT((height % inf.m_blockHeight) == 0);
		return (width / inf.m_blockWidth) * (height / inf.m_blockHeight) * inf.m_blockSize * depth;
	}
	else
	{
		return width * height * depth * inf.m_texelSize;
	}
}

U32 computeMaxMipmapCount2d(U32 w, U32 h, U32 minSizeOfLastMip)
{
	ANKI_ASSERT(w >= minSizeOfLastMip && h >= minSizeOfLastMip);
	U32 s = (w < h) ? w : h;
	U32 count = 0;
	while(s >= minSizeOfLastMip)
	{
		s /= 2;
		++count;
	}

	return count;
}

/// Compute max number of mipmaps for a 3D texture.
U32 computeMaxMipmapCount3d(U32 w, U32 h, U32 d, U32 minSizeOfLastMip)
{
	U32 s = (w < h) ? w : h;
	s = (s < d) ? s : d;
	U32 count = 0;
	while(s >= minSizeOfLastMip)
	{
		s /= 2;
		++count;
	}

	return count;
}

Error ShaderReflection::linkShaderReflection(const ShaderReflection& a, const ShaderReflection& b, ShaderReflection& c_)
{
	ShaderReflection c;

	memcpy(&c.m_descriptor, &a.m_descriptor, sizeof(a.m_descriptor));
	for(U32 set = 0; set < kMaxDescriptorSets; ++set)
	{
		for(U32 binding = 0; binding < b.m_descriptor.m_bindingCounts[set]; ++binding)
		{
			// Search for the binding in a
			const ShaderReflectionBinding& bbinding = b.m_descriptor.m_bindings[set][binding];
			Bool bindingFoundOnA = false;
			for(U32 binding2 = 0; binding2 < a.m_descriptor.m_bindingCounts[set]; ++binding2)
			{
				const ShaderReflectionBinding& abinding = a.m_descriptor.m_bindings[set][binding2];

				if(abinding.m_registerBindingPoint == bbinding.m_registerBindingPoint
				   && descriptorTypeToHlslResourceType(abinding.m_type, abinding.m_flags)
						  == descriptorTypeToHlslResourceType(bbinding.m_type, bbinding.m_flags))
				{
					if(abinding != bbinding)
					{
						ANKI_GR_LOGE("Can't link shader reflection because of different bindings. Set %u binding %u", set, binding);
						return Error::kFunctionFailed;
					}
					bindingFoundOnA = true;
					break;
				}
			}

			if(!bindingFoundOnA)
			{
				c.m_descriptor.m_bindings[set][c.m_descriptor.m_bindingCounts[set]++] = bbinding;
			}
		}
	}

	if(a.m_descriptor.m_pushConstantsSize != 0 && b.m_descriptor.m_pushConstantsSize != 0
	   && a.m_descriptor.m_pushConstantsSize != b.m_descriptor.m_pushConstantsSize)
	{
		ANKI_GR_LOGE("Can't link shader reflection because push constant size doesn't match");
		return Error::kFunctionFailed;
	}
	c.m_descriptor.m_pushConstantsSize = max(a.m_descriptor.m_pushConstantsSize, b.m_descriptor.m_pushConstantsSize);

	c.m_descriptor.m_hasVkBindlessDescriptorSet = a.m_descriptor.m_hasVkBindlessDescriptorSet || b.m_descriptor.m_hasVkBindlessDescriptorSet;

	c.m_vertex.m_vkVertexAttributeLocations =
		(a.m_vertex.m_vertexAttributeMask.getAnySet()) ? a.m_vertex.m_vkVertexAttributeLocations : b.m_vertex.m_vkVertexAttributeLocations;
	c.m_vertex.m_vertexAttributeMask = a.m_vertex.m_vertexAttributeMask | b.m_vertex.m_vertexAttributeMask;

	c.m_fragment.m_colorAttachmentWritemask = a.m_fragment.m_colorAttachmentWritemask | b.m_fragment.m_colorAttachmentWritemask;
	c.m_fragment.m_discards = a.m_fragment.m_discards || b.m_fragment.m_discards;

	c_ = c;
	return Error::kNone;
}

} // end namespace anki
