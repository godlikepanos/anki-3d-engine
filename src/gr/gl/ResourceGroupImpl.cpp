// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/gl/ResourceGroupImpl.h"
#include "anki/gr/TexturePtr.h"
#include "anki/gr/gl/TextureImpl.h"
#include "anki/gr/gl/SamplerImpl.h"
#include "anki/gr/gl/BufferImpl.h"
#include "anki/gr/gl/GlState.h"
#include "anki/gr/gl/GrManagerImpl.h"
#include "anki/gr/gl/RenderingThread.h"

namespace anki {

//==============================================================================
Error ResourceGroupImpl::create(const Initializer& init)
{
	static_cast<Initializer&>(*this) = init;

	// Init textures & samplers
	m_textureNamesCount = 0;
	m_allSamplersZero = true;
	for(U i = 0; i < MAX_TEXTURE_BINDINGS; ++i)
	{
		if(m_textures[i].m_texture.isCreated())
		{
			m_textureNames[i] = m_textures[i].m_texture.get().getGlName();
			m_textureNamesCount = i + 1;
		}
		else
		{
			m_textureNames[i] = 0;
		}

		if(m_textures[i].m_sampler.isCreated())
		{
			m_samplerNames[i] = m_textures[i].m_sampler.get().getGlName();
			m_allSamplersZero = false;
		}
		else
		{
			m_samplerNames[i] = 0;
		}
	}

	// Init buffers
	m_ubosCount = 0;
	for(U i = 0; i < MAX_UNIFORM_BUFFER_BINDINGS; ++i)
	{
		if(m_uniformBuffers[i].m_buffer.isCreated())
		{
			m_ubosCount = i + 1;
		}
	}

	m_ssbosCount = 0;
	for(U i = 0; i < MAX_STORAGE_BUFFER_BINDINGS; ++i)
	{
		if(m_storageBuffers[i].m_buffer.isCreated())
		{
			m_ssbosCount = i + 1;
		}
	}

	// Init vert buffers
	m_vertBindingsCount = 0;
	for(U i = 0; i < MAX_VERTEX_ATTRIBUTES; ++i)
	{
		const BufferBinding& binding = m_vertexBuffers[i];
		if(binding.m_buffer.isCreated())
		{
			m_vertBuffNames[i] = binding.m_buffer.get().getGlName();
			m_vertBuffOffsets[i] = binding.m_offset;

			m_vertBindingsCount = i + 1;
		}
		else
		{
			m_vertBuffNames[i] = 0;
			m_vertBuffOffsets[i] = 0;
		}
	}

	return ErrorCode::NONE;
}

//==============================================================================
void ResourceGroupImpl::bind()
{
	// Bind textures
	if(m_textureNamesCount)
	{
		glBindTextures(0, m_textureNamesCount, &m_textureNames[0]);

		if(m_allSamplersZero)
		{
			glBindSamplers(0, m_textureNamesCount, nullptr);
		}
		else
		{
			glBindSamplers(0, m_textureNamesCount, &m_samplerNames[0]);
		}
	}

	// Uniform buffers
	for(U i = 0; i < m_ubosCount; ++i)
	{
		BufferBinding& binding = m_uniformBuffers[i];

		if(binding.m_buffer.isCreated())
		{
			BufferImpl& buff = binding.m_buffer.get();
			if(buff.getTarget() != GL_UNIFORM_BUFFER)
			{
				buff.setTarget(GL_UNIFORM_BUFFER);
			}
			buff.setBindingRange(i, binding.m_offset, binding.m_range);
		}
	}

	// Storage buffers
	for(U i = 0; i < m_ssbosCount; ++i)
	{
		BufferBinding& binding = m_storageBuffers[i];

		if(binding.m_buffer.isCreated())
		{
			BufferImpl& buff = binding.m_buffer.get();
			if(buff.getTarget() != GL_SHADER_STORAGE_BUFFER)
			{
				buff.setTarget(GL_SHADER_STORAGE_BUFFER);
			}
			buff.setBindingRange(i, binding.m_offset, binding.m_range);
		}
	}

	// Vertex buffers
	if(m_vertBindingsCount)
	{
		const GlState& state =
			getManager().getImplementation().getRenderingThread().getState();

		glBindVertexBuffers(
			0, m_vertBindingsCount, &m_vertBuffNames[0], &m_vertBuffOffsets[0],
			&state.m_vertexBindingStrides[0]);
	}

	// Index buffer
	if(m_indexBuffer.m_buffer.isCreated())
	{
		BufferImpl& buff = m_indexBuffer.m_buffer.get();
		if(buff.getTarget() != GL_ELEMENT_ARRAY_BUFFER)
		{
			buff.setTarget(GL_ELEMENT_ARRAY_BUFFER);
		}
		buff.bind();
	}
}

} // end namespace anki
