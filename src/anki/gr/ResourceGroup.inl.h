// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/ResourceGroup.h>

namespace anki
{

inline void ResourceGroupInitInfo::appendBufferBinding(const BufferBinding& b, U64 arr[], U& count) const
{
	arr[count++] = (b.m_buffer) ? b.m_buffer->getUuid() : 0;
	arr[count++] = b.m_offset;
	arr[count++] = b.m_range;
	arr[count++] = b.m_uploadedMemory;
	arr[count++] = static_cast<U64>(b.m_usage);
}

inline U64 ResourceGroupInitInfo::computeHash() const
{
	const U TEX_NUMBERS = MAX_TEXTURE_BINDINGS * 4;
	const U BUFF_NUMBERS = (MAX_UNIFORM_BUFFER_BINDINGS + MAX_STORAGE_BUFFER_BINDINGS + MAX_VERTEX_ATTRIBUTES + 1) * 5;
	const U IMAGE_NUMBERS = MAX_IMAGE_BINDINGS * 3;
	const U INDEX_SIZE_NUMBERS = 1;
	Array<U64, TEX_NUMBERS + BUFF_NUMBERS + IMAGE_NUMBERS + INDEX_SIZE_NUMBERS> numbers;
	U count = 0;

	for(const auto& tex : m_textures)
	{
		if(tex.m_texture)
		{
			numbers[count++] = tex.m_texture->getUuid();
			if(tex.m_sampler)
			{
				numbers[count++] = tex.m_sampler->getUuid();
			}
			numbers[count++] = static_cast<U64>(tex.m_usage);
			numbers[count++] = static_cast<U64>(tex.m_aspect);
		}
		else
		{
			break;
		}
	}

	for(const auto& b : m_uniformBuffers)
	{
		if(b.m_buffer || b.m_uploadedMemory)
		{
			appendBufferBinding(b, &numbers[0], count);
		}
		else
		{
			break;
		}
	}
	for(const auto& b : m_storageBuffers)
	{
		if(b.m_buffer || b.m_uploadedMemory)
		{
			appendBufferBinding(b, &numbers[0], count);
		}
		else
		{
			break;
		}
	}
	for(const auto& b : m_vertexBuffers)
	{
		if(b.m_buffer || b.m_uploadedMemory)
		{
			appendBufferBinding(b, &numbers[0], count);
		}
		else
		{
			break;
		}
	}
	appendBufferBinding(m_indexBuffer, &numbers[0], count);

	for(const auto& img : m_images)
	{
		if(img.m_texture)
		{
			numbers[count++] = img.m_texture->getUuid();
			numbers[count++] = img.m_level;
			numbers[count++] = static_cast<U64>(img.m_usage);
		}
		else
		{
			break;
		}
	}

	numbers[count++] = static_cast<U64>(m_indexSize);

	return anki::computeHash(&numbers[0], count * sizeof(U64), 458);
}

} // end namespace anki
