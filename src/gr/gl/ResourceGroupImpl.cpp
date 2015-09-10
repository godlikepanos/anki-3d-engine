// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/gl/ResourceGroupImpl.h"
#include "anki/gr/gl/TextureImpl.h"
#include "anki/gr/Texture.h"
#include "anki/gr/gl/SamplerImpl.h"
#include "anki/gr/Sampler.h"
#include "anki/gr/gl/BufferImpl.h"
#include "anki/gr/Buffer.h"
#include "anki/gr/gl/GlState.h"
#include "anki/gr/gl/GrManagerImpl.h"
#include "anki/gr/GrManager.h"
#include "anki/gr/gl/RenderingThread.h"

namespace anki {

//==============================================================================
template<typename InBindings, typename OutBindings>
void ResourceGroupImpl::initBuffers(
	const InBindings& in, OutBindings& out, U8& count, U& resourcesCount)
{
	count = 0;

	for(U i = 0; i < in.getSize(); ++i)
	{
		const BufferBinding& binding = in[i];

		if(binding.m_buffer.isCreated())
		{
			const BufferImpl& buff = binding.m_buffer->getImplementation();
			InternalBufferBinding& outBinding = out[count];

			outBinding.m_name = buff.getGlName();
			outBinding.m_offset = binding.m_offset;
			outBinding.m_range =
				(binding.m_range != 0)
				? binding.m_range
				: (buff.m_size - binding.m_offset);

			ANKI_ASSERT(
				outBinding.m_offset + outBinding.m_range <= buff.m_size);
			ANKI_ASSERT(outBinding.m_range > 0);

			++count;
			++resourcesCount;
		}
	}
}

//==============================================================================
void ResourceGroupImpl::create(const ResourceGroupInitializer& init)
{
	U resourcesCount = 0;

	// Init textures & samplers
	m_textureNamesCount = 0;
	m_allSamplersZero = true;
	for(U i = 0; i < init.m_textures.getSize(); ++i)
	{
		if(init.m_textures[i].m_texture.isCreated())
		{
			m_textureNames[i] =
				init.m_textures[i].m_texture->getImplementation().getGlName();
			++m_textureNamesCount;
			++resourcesCount;
		}
		else
		{
			m_textureNames[i] = 0;
		}

		if(init.m_textures[i].m_sampler.isCreated())
		{
			m_samplerNames[i] =
				init.m_textures[i].m_sampler->getImplementation().getGlName();
			m_allSamplersZero = false;
			++resourcesCount;
		}
		else
		{
			m_samplerNames[i] = 0;
		}
	}

	// Init buffers
	initBuffers(init.m_uniformBuffers, m_ubos, m_ubosCount, resourcesCount);
	initBuffers(init.m_storageBuffers, m_ssbos, m_ssbosCount, resourcesCount);

	// Init vert buffers
	m_vertBindingsCount = 0;
	for(U i = 0; i < init.m_vertexBuffers.getSize(); ++i)
	{
		const BufferBinding& binding = init.m_vertexBuffers[i];
		if(binding.m_buffer.isCreated())
		{
			m_vertBuffNames[i] =
				binding.m_buffer->getImplementation().getGlName();
			m_vertBuffOffsets[i] = binding.m_offset;

			++m_vertBindingsCount;
			++resourcesCount;
		}
		else
		{
			m_vertBuffNames[i] = 0;
			m_vertBuffOffsets[i] = 0;
		}
	}

	// Init index buffer
	if(init.m_indexBuffer.m_buffer.isCreated())
	{
		const BufferImpl& buff =
			init.m_indexBuffer.m_buffer->getImplementation();

		ANKI_ASSERT(init.m_indexSize == 2 || init.m_indexSize == 4);

		m_indexBuffName = buff.getGlName();
		m_indexSize = init.m_indexSize;
		++resourcesCount;
	}

	ANKI_ASSERT(resourcesCount > 0 && "Resource group empty");

	// Hold references
	initResourceReferences(init, resourcesCount);
}

//==============================================================================
void ResourceGroupImpl::initResourceReferences(
	const ResourceGroupInitializer& init, U refCount)
{
	m_refs.create(getAllocator(), refCount);

	U count = 0;

	for(U i = 0; i < init.m_textures.getSize(); ++i)
	{
		if(init.m_textures[i].m_texture.isCreated())
		{
			m_refs[count++] = init.m_textures[i].m_texture;
		}

		if(init.m_textures[i].m_sampler.isCreated())
		{
			m_refs[count++] = init.m_textures[i].m_sampler;
		}
	}

	for(U i = 0; i < init.m_uniformBuffers.getSize(); ++i)
	{
		const BufferBinding& binding = init.m_uniformBuffers[i];
		if(binding.m_buffer.isCreated())
		{
			m_refs[count++] = binding.m_buffer;
		}
	}

	for(U i = 0; i < init.m_storageBuffers.getSize(); ++i)
	{
		const BufferBinding& binding = init.m_storageBuffers[i];
		if(binding.m_buffer.isCreated())
		{
			m_refs[count++] = binding.m_buffer;
		}
	}

	for(U i = 0; i < init.m_vertexBuffers.getSize(); ++i)
	{
		const BufferBinding& binding = init.m_vertexBuffers[i];
		if(binding.m_buffer.isCreated())
		{
			m_refs[count++] = binding.m_buffer;
		}
	}

	if(init.m_indexBuffer.m_buffer.isCreated())
	{
		m_refs[count++] = init.m_indexBuffer.m_buffer;
	}

	ANKI_ASSERT(refCount == count);
}

//==============================================================================
void ResourceGroupImpl::bind(U slot, GlState& state)
{
	ANKI_ASSERT(slot < MAX_RESOURCE_GROUPS);

	// Bind textures
	if(m_textureNamesCount)
	{
		glBindTextures(MAX_TEXTURE_BINDINGS * slot, m_textureNamesCount,
			&m_textureNames[0]);

		if(m_allSamplersZero)
		{
			glBindSamplers(MAX_TEXTURE_BINDINGS * slot, m_textureNamesCount,
				nullptr);
		}
		else
		{
			glBindSamplers(MAX_TEXTURE_BINDINGS * slot, m_textureNamesCount,
				&m_samplerNames[0]);
		}
	}

	// Uniform buffers
	for(U i = 0; i < m_ubosCount; ++i)
	{
		const auto& binding = m_ubos[i];
		glBindBufferRange(GL_UNIFORM_BUFFER,
			MAX_UNIFORM_BUFFER_BINDINGS * slot + i, binding.m_name,
			binding.m_offset, binding.m_range);
	}

	// Storage buffers
	for(U i = 0; i < m_ssbosCount; ++i)
	{
		const auto& binding = m_ssbos[i];
		glBindBufferRange(GL_SHADER_STORAGE_BUFFER,
			MAX_STORAGE_BUFFER_BINDINGS * slot + i, binding.m_name,
			binding.m_offset, binding.m_range);
	}

	// Vertex buffers
	if(m_vertBindingsCount)
	{
		ANKI_ASSERT(slot == 0 && "Only slot 0 can have vertex buffers");
		glBindVertexBuffers(
			0, m_vertBindingsCount, &m_vertBuffNames[0],
			&m_vertBuffOffsets[0], &state.m_vertexBindingStrides[0]);
	}

	// Index buffer
	if(m_indexSize > 0)
	{
		ANKI_ASSERT(slot == 0 && "Only slot 0 can have index buffers");
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffName);
		state.m_indexSize = m_indexSize;
	}
}

} // end namespace anki
