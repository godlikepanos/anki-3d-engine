// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/gl/ResourceGroupImpl.h>
#include <anki/gr/gl/TextureImpl.h>
#include <anki/gr/Texture.h>
#include <anki/gr/gl/SamplerImpl.h>
#include <anki/gr/Sampler.h>
#include <anki/gr/gl/BufferImpl.h>
#include <anki/gr/Buffer.h>
#include <anki/gr/gl/GlState.h>
#include <anki/gr/gl/GrManagerImpl.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/gl/RenderingThread.h>
#include <anki/gr/gl/CommandBufferImpl.h>
#include <anki/gr/gl/TransientMemoryManager.h>

namespace anki
{

//==============================================================================
template<typename InBindings, typename OutBindings>
void ResourceGroupImpl::initBuffers(const InBindings& in,
	OutBindings& out,
	U8& count,
	U& resourcesCount,
	U& transCount)
{
	count = 0;

	for(U i = 0; i < in.getSize(); ++i)
	{
		const BufferBinding& binding = in[i];

		if(binding.m_buffer.isCreated())
		{
			ANKI_ASSERT(binding.m_uploadedMemory == false);

			const BufferImpl& buff = binding.m_buffer->getImplementation();
			InternalBufferBinding& outBinding = out[count];

			outBinding.m_name = buff.getGlName();
			outBinding.m_offset = binding.m_offset;
			outBinding.m_range = (binding.m_range != 0)
				? binding.m_range
				: (buff.m_size - binding.m_offset);

			ANKI_ASSERT(
				outBinding.m_offset + outBinding.m_range <= buff.m_size);
			ANKI_ASSERT(outBinding.m_range > 0);

			++resourcesCount;
			count = i + 1;
		}
		else if(binding.m_uploadedMemory)
		{
			InternalBufferBinding& outBinding = out[count];
			outBinding.m_name = MAX_U32;
			++transCount;
			count = i + 1;
		}
	}
}

//==============================================================================
void ResourceGroupImpl::init(const ResourceGroupInitInfo& init)
{
	U resourcesCount = 0;
	U transCount = 0;

	// Init textures & samplers
	m_textureNamesCount = 0;
	m_allSamplersZero = true;
	for(U i = 0; i < init.m_textures.getSize(); ++i)
	{
		if(init.m_textures[i].m_texture.isCreated())
		{
			m_textureNames[i] =
				init.m_textures[i].m_texture->getImplementation().getGlName();
			m_textureNamesCount = i + 1;
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

	// Init shader buffers
	initBuffers(
		init.m_uniformBuffers, m_ubos, m_ubosCount, resourcesCount, transCount);
	initBuffers(init.m_storageBuffers,
		m_ssbos,
		m_ssbosCount,
		resourcesCount,
		transCount);

	// Init images
	for(U i = 0; i < MAX_IMAGE_BINDINGS; ++i)
	{
		const auto& in = init.m_images[i];
		if(in.m_texture)
		{
			TextureImpl& impl = in.m_texture->getImplementation();
			impl.checkSurface(TextureSurfaceInfo(in.m_level, 0, 0, 0));

			ImageBinding& out = m_images[i];

			out.m_name = in.m_texture->getImplementation().getGlName();
			out.m_level = in.m_level;
			out.m_format = impl.m_internalFormat;

			++m_imageCount;
			++resourcesCount;
		}
	}

	// Init vert buffers
	m_vertBindingsCount = 0;
	for(U i = 0; i < init.m_vertexBuffers.getSize(); ++i)
	{
		const BufferBinding& binding = init.m_vertexBuffers[i];
		if(binding.m_buffer.isCreated())
		{
			ANKI_ASSERT(!binding.m_uploadedMemory);

			m_vertBuffNames[i] =
				binding.m_buffer->getImplementation().getGlName();
			m_vertBuffOffsets[i] = binding.m_offset;

			++m_vertBindingsCount;
			++resourcesCount;
		}
		else if(binding.m_uploadedMemory)
		{
			++transCount;

			m_vertBuffNames[i] = 0;
			m_vertBuffOffsets[i] = MAX_U32;

			++m_vertBindingsCount;
			m_hasTransientVertexBuff = true;
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

	ANKI_ASSERT(
		(resourcesCount > 0 || transCount > 0) && "Resource group empty");

	// Hold references
	initResourceReferences(init, resourcesCount);
}

//==============================================================================
void ResourceGroupImpl::initResourceReferences(
	const ResourceGroupInitInfo& init, U refCount)
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

	for(U i = 0; i < MAX_IMAGE_BINDINGS; ++i)
	{
		const auto& binding = init.m_images[i];
		if(binding.m_texture)
		{
			m_refs[count++] = binding.m_texture;
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
void ResourceGroupImpl::bind(
	U slot, const TransientMemoryInfo& transientInfo, GlState& state)
{
	ANKI_ASSERT(slot < MAX_BOUND_RESOURCE_GROUPS);

	// Bind textures
	if(m_textureNamesCount)
	{
		glBindTextures(MAX_TEXTURE_BINDINGS * slot,
			m_textureNamesCount,
			&m_textureNames[0]);

		if(m_allSamplersZero)
		{
			glBindSamplers(
				MAX_TEXTURE_BINDINGS * slot, m_textureNamesCount, nullptr);
		}
		else
		{
			glBindSamplers(MAX_TEXTURE_BINDINGS * slot,
				m_textureNamesCount,
				&m_samplerNames[0]);
		}
	}

	// Uniform buffers
	for(U i = 0; i < m_ubosCount; ++i)
	{
		const auto& binding = m_ubos[i];
		if(binding.m_name == MAX_U32)
		{
			// Transient
			TransientMemoryToken token = transientInfo.m_uniformBuffers[i];
			ANKI_ASSERT(token.m_range != 0);

			if(token.m_range != MAX_U32)
			{
				glBindBufferRange(GL_UNIFORM_BUFFER,
					MAX_UNIFORM_BUFFER_BINDINGS * slot + i,
					getManager()
						.getImplementation()
						.getTransientMemoryManager()
						.getGlName(token),
					token.m_offset,
					token.m_range);
			}
			else
			{
				// It's invalid
			}
		}
		else if(binding.m_name != 0)
		{
			// Static
			glBindBufferRange(GL_UNIFORM_BUFFER,
				MAX_UNIFORM_BUFFER_BINDINGS * slot + i,
				binding.m_name,
				binding.m_offset,
				binding.m_range);
		}
	}

	// Storage buffers
	for(U i = 0; i < m_ssbosCount; ++i)
	{
		const auto& binding = m_ssbos[i];
		if(binding.m_name == MAX_U32)
		{
			// Transient
			TransientMemoryToken token = transientInfo.m_storageBuffers[i];
			ANKI_ASSERT(token.m_range != 0);

			if(token.m_range != MAX_U32)
			{
				glBindBufferRange(GL_SHADER_STORAGE_BUFFER,
					MAX_STORAGE_BUFFER_BINDINGS * slot + i,
					getManager()
						.getImplementation()
						.getTransientMemoryManager()
						.getGlName(token),
					token.m_offset,
					token.m_range);
			}
			else
			{
				// It's invalid
			}
		}
		else if(binding.m_name != 0)
		{
			// Static
			glBindBufferRange(GL_SHADER_STORAGE_BUFFER,
				MAX_STORAGE_BUFFER_BINDINGS * slot + i,
				binding.m_name,
				binding.m_offset,
				binding.m_range);
		}
	}

	// Images
	for(U i = 0; i < m_imageCount; ++i)
	{
		const ImageBinding& binding = m_images[i];
		if(binding.m_name)
		{
			glBindImageTexture(MAX_IMAGE_BINDINGS * slot + i,
				binding.m_name,
				binding.m_level,
				GL_TRUE,
				0,
				GL_READ_WRITE,
				binding.m_format);
		}
	}

	// Vertex buffers
	if(m_vertBindingsCount)
	{
		ANKI_ASSERT(slot == 0 && "Only slot 0 can have vertex buffers");

		if(!m_hasTransientVertexBuff)
		{
			memcpy(&state.m_vertBuffOffsets[0],
				&m_vertBuffOffsets[0],
				sizeof(m_vertBuffOffsets[0]) * m_vertBindingsCount);

			memcpy(&state.m_vertBuffNames[0],
				&m_vertBuffNames[0],
				sizeof(m_vertBuffNames[0]) * m_vertBindingsCount);
		}
		else
		{
			// Copy the offsets
			Array<GLintptr, MAX_VERTEX_ATTRIBUTES> offsets = m_vertBuffOffsets;
			Array<GLuint, MAX_VERTEX_ATTRIBUTES> names = m_vertBuffNames;

			for(U i = 0; i < MAX_VERTEX_ATTRIBUTES; ++i)
			{
				if(offsets[i] == MAX_U32)
				{
					// It's dynamic
					ANKI_ASSERT(transientInfo.m_vertexBuffers[i].m_range != 0);
					offsets[i] = transientInfo.m_vertexBuffers[i].m_offset;
					names[i] = getManager()
								   .getImplementation()
								   .getTransientMemoryManager()
								   .getGlName(transientInfo.m_vertexBuffers[i]);
				}
				else
				{
					ANKI_ASSERT(transientInfo.m_vertexBuffers[i].m_range == 0);
				}
			}

			// Bind to state
			memcpy(&state.m_vertBuffOffsets[0],
				&offsets[0],
				sizeof(offsets[0]) * m_vertBindingsCount);

			memcpy(
				&names[0], &names[0], sizeof(names[0]) * m_vertBindingsCount);
		}

		state.m_vertBindingCount = m_vertBindingsCount;
		state.m_vertBindingsDirty = true;
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
