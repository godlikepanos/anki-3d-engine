// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/SpirvReflection.h>
#include <anki/util/List.h>
#include <SPIRV/spirv.hpp>

namespace anki
{

using Word = U32;

class SpirvHeader
{
public:
	Word m_magic;
	Word m_version;
	Word m_generatorMagic;
	Word m_idCount;
	Word m_reserved;
};

class Variable
{
public:
	Word m_id = 0;
	U8 m_set = MAX_U8;
	U8 m_binding = MAX_U8;
};

Error SpirvReflection::parse()
{
	ListAuto<Variable> variables(m_alloc);

	// For all instructions
	U counter = sizeof(SpirvHeader) / sizeof(Word);
	while(counter < m_spv.getSize())
	{
		Word combo = m_spv[counter];
		spv::Op instruction = spv::Op(combo & 0xFFFF);
		Word instructionWordCount = combo >> 16;

		// If decorate
		if(instruction == spv::OpDecorate)
		{
			U c = counter + 1;
			Word rezId = m_spv[c++];
			spv::Decoration decoration = spv::Decoration(m_spv[c++]);

			if(decoration == spv::Decoration::DecorationDescriptorSet
				|| decoration == spv::Decoration::DecorationBinding)
			{
				// Find or create variable
				Variable* var = nullptr;
				for(Variable& v : variables)
				{
					if(v.m_id == rezId)
					{
						var = &v;
						break;
					}
				}

				if(var == nullptr)
				{
					Variable v;
					v.m_id = rezId;
					variables.pushBack(v);
					var = &variables.getBack();
				}

				// Check the decoration
				if(decoration == spv::Decoration::DecorationDescriptorSet)
				{
					Word set = m_spv[c];
					if(set >= MAX_BOUND_RESOURCE_GROUPS)
					{
						ANKI_LOGE("Cannot accept shaders with descriptor set >= to %u", MAX_BOUND_RESOURCE_GROUPS);
						return ErrorCode::USER_DATA;
					}

					m_setMask |= 1 << set;
					ANKI_ASSERT(var->m_set == MAX_U8);
					var->m_set = set;
				}
				else
				{
					ANKI_ASSERT(decoration == spv::Decoration::DecorationBinding);
					Word binding = m_spv[c];

					ANKI_ASSERT(var->m_binding == MAX_U8);
					var->m_binding = binding;
				}
			}
		}

		counter += instructionWordCount;
	}

	// Iterate the variables
	for(const Variable& v : variables)
	{
		if(v.m_set == MAX_U8 || v.m_binding == MAX_U8)
		{
			ANKI_LOGE("Missing set or binding decoration of ID %u", v.m_id);
			return ErrorCode::USER_DATA;
		}

		const U32 binding = v.m_binding;
		const U set = v.m_set;
		if(binding < MAX_TEXTURE_BINDINGS)
		{
			m_texBindings[set] = max(m_texBindings[set], binding + 1);
		}
		else if(binding < MAX_TEXTURE_BINDINGS + MAX_UNIFORM_BUFFER_BINDINGS)
		{
			m_uniBindings[set] = max<U>(m_uniBindings[set], binding + 1 - MAX_TEXTURE_BINDINGS);
		}
		else if(binding < MAX_TEXTURE_BINDINGS + MAX_UNIFORM_BUFFER_BINDINGS + MAX_STORAGE_BUFFER_BINDINGS)
		{
			m_storageBindings[set] =
				max<U>(m_storageBindings[set], binding + 1 - (MAX_TEXTURE_BINDINGS + MAX_UNIFORM_BUFFER_BINDINGS));
		}
		else
		{
			ANKI_ASSERT(binding < MAX_TEXTURE_BINDINGS + MAX_UNIFORM_BUFFER_BINDINGS + MAX_STORAGE_BUFFER_BINDINGS
					+ MAX_IMAGE_BINDINGS);
			m_imageBindings[set] = max<U>(m_imageBindings[set],
				binding + 1 - (MAX_TEXTURE_BINDINGS + MAX_UNIFORM_BUFFER_BINDINGS + MAX_STORAGE_BUFFER_BINDINGS));
		}
	}

	return ErrorCode::NONE;
}

} // end namespace anki
