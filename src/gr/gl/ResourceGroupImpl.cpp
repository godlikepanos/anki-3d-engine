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
void ResourceGroupImpl::create(const ResourceGroupInitializer& init)
{
	m_in = init;

	// Init textures & samplers
	m_cache.m_textureNamesCount = 0;
	m_cache.m_allSamplersZero = true;
	for(U i = 0; i < MAX_TEXTURE_BINDINGS; ++i)
	{
		if(m_in.m_textures[i].m_texture.isCreated())
		{
			m_cache.m_textureNames[i] =
				m_in.m_textures[i].m_texture->getImplementation().getGlName();
			++m_cache.m_textureNamesCount;
		}
		else
		{
			m_cache.m_textureNames[i] = 0;
		}

		if(m_in.m_textures[i].m_sampler.isCreated())
		{
			m_cache.m_samplerNames[i] =
				m_in.m_textures[i].m_sampler->getImplementation().getGlName();
			m_cache.m_allSamplersZero = false;
		}
		else
		{
			m_cache.m_samplerNames[i] = 0;
		}
	}

	// Init buffers
	m_cache.m_ubosCount = 0;
	for(U i = 0; i < MAX_UNIFORM_BUFFER_BINDINGS; ++i)
	{
		if(m_in.m_uniformBuffers[i].m_buffer.isCreated())
		{
			++m_cache.m_ubosCount;
		}
	}

	m_cache.m_ssbosCount = 0;
	for(U i = 0; i < MAX_STORAGE_BUFFER_BINDINGS; ++i)
	{
		if(m_in.m_storageBuffers[i].m_buffer.isCreated())
		{
			++m_cache.m_ssbosCount;
		}
	}

	// Init vert buffers
	m_cache.m_vertBindingsCount = 0;
	for(U i = 0; i < MAX_VERTEX_ATTRIBUTES; ++i)
	{
		const BufferBinding& binding = m_in.m_vertexBuffers[i];
		if(binding.m_buffer.isCreated())
		{
			m_cache.m_vertBuffNames[i] =
				binding.m_buffer->getImplementation().getGlName();
			m_cache.m_vertBuffOffsets[i] = binding.m_offset;

			++m_cache.m_vertBindingsCount;
		}
		else
		{
			m_cache.m_vertBuffNames[i] = 0;
			m_cache.m_vertBuffOffsets[i] = 0;
		}
	}
}

//==============================================================================
void ResourceGroupImpl::bind(GlState& state)
{
	// Bind textures
	if(m_cache.m_textureNamesCount)
	{
		glBindTextures(
			0, m_cache.m_textureNamesCount, &m_cache.m_textureNames[0]);

		if(m_cache.m_allSamplersZero)
		{
			glBindSamplers(0, m_cache.m_textureNamesCount, nullptr);
		}
		else
		{
			glBindSamplers(
				0, m_cache.m_textureNamesCount, &m_cache.m_samplerNames[0]);
		}
	}

	// Uniform buffers
	for(U i = 0; i < m_cache.m_ubosCount; ++i)
	{
		const BufferBinding& binding = m_in.m_uniformBuffers[i];

		if(binding.m_buffer.isCreated())
		{
			const BufferImpl& buff = binding.m_buffer->getImplementation();
			buff.bind(GL_UNIFORM_BUFFER, i, binding.m_offset, binding.m_range);
		}
	}

	// Storage buffers
	for(U i = 0; i < m_cache.m_ssbosCount; ++i)
	{
		BufferBinding& binding = m_in.m_storageBuffers[i];

		if(binding.m_buffer.isCreated())
		{
			BufferImpl& buff = binding.m_buffer->getImplementation();
			buff.bind(
				GL_SHADER_STORAGE_BUFFER, i, binding.m_offset, binding.m_range);
		}
	}

	// Vertex buffers
	if(m_cache.m_vertBindingsCount)
	{
		glBindVertexBuffers(
			0, m_cache.m_vertBindingsCount, &m_cache.m_vertBuffNames[0],
			&m_cache.m_vertBuffOffsets[0], &state.m_vertexBindingStrides[0]);
	}

	// Index buffer
	if(m_in.m_indexBuffer.m_buffer.isCreated())
	{
		const BufferImpl& buff =
			m_in.m_indexBuffer.m_buffer->getImplementation();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buff.getGlName());

		ANKI_ASSERT(m_in.m_indexSize == 2 || m_in.m_indexSize == 4);
		state.m_indexSize = m_in.m_indexSize;
	}
}

} // end namespace anki
