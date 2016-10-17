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
	U counter = 0;
	while(counter < m_spv.getSize())
	{
		Word combo = m_spv[counter];
		Word instruction = combo & 0xFFFF;
		Word instrctionWordCount = combo >> 16;

		// If decorate
		if(instruction == spv::OpDecorate)
		{
			U c = counter + sizeof(Word);

			// Find or create variable
			Word rezId = m_spv[c++];
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
			Word decoration = m_spv[c++];
			if(decoration == Word(spv::Decoration::DecorationDescriptorSet))
			{
				Word set = m_spv[c];
				if(set >= MAX_RESOURCE_GROUPS)
				{
					ANKI_LOGE("Cannot accept shaders with descriptor set >= to %u", MAX_RESOURCE_GROUPS);
					return ErrorCode::USER_DATA;
				}

				m_setMask |= 1 << set;
				ANKI_ASSERT(var.m_set == MAX_U8);
				var->m_set = set;
			}
			else if(decoration == Word(spv::Decoration::DecorationBinding))
			{
				Word binding = m_spv[c];

				ANKI_ASSERT(var.m_binding == MAX_U8);
				var->m_binding = binding;
			}
		}

		counter += instrctionWordCount;
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
			m_uniBindings[set] = max(m_uniBindings[set], binding + 1);
		}
		else if(binding < MAX_TEXTURE_BINDINGS + MAX_UNIFORM_BUFFER_BINDINGS + MAX_STORAGE_BUFFER_BINDINGS)
		{
			m_storageBindings[set] = max(m_storageBindings[set], binding + 1);
		}
		else
		{
			ANKI_ASSERT(binding < MAX_TEXTURE_BINDINGS + MAX_UNIFORM_BUFFER_BINDINGS + MAX_STORAGE_BUFFER_BINDINGS
					+ MAX_IMAGE_BINDINGS);
			m_imageBindings[set] = max(m_imageBindings[set], binding + 1);
		}
	}

	return ErrorCode::NONE;
}

} // end namespace anki
