// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/GrObject.h>
#include <anki/gr/Texture.h>
#include <anki/gr/Sampler.h>
#include <anki/gr/Buffer.h>

namespace anki {

/// @addtogroup graphics
/// @{

/// Texture/Sampler Binding.
class TextureBinding
{
public:
	TexturePtr m_texture;
	SamplerPtr m_sampler; ///< Use it to override texture's sampler.
};

/// Buffer binding info.
class BufferBinding
{
public:
	BufferPtr m_buffer;
	PtrSize m_offset = 0;
	PtrSize m_range = 0;
	Bool m_dynamic = false;
};

/// Resource group initializer.
class ResourceGroupInitializer
{
public:
	Array<TextureBinding, MAX_TEXTURE_BINDINGS> m_textures;
	Array<BufferBinding, MAX_UNIFORM_BUFFER_BINDINGS> m_uniformBuffers;
	Array<BufferBinding, MAX_STORAGE_BUFFER_BINDINGS> m_storageBuffers;
	Array<BufferBinding, MAX_VERTEX_ATTRIBUTES> m_vertexBuffers;
	BufferBinding m_indexBuffer;
	I8 m_indexSize = -1; ///< Index size in bytes. 2 or 4
};

/// Struct to help update the offset of the dynamic buffers.
class DynamicBufferInfo
{
public:
	Array<DynamicBufferToken, MAX_UNIFORM_BUFFER_BINDINGS> m_uniformBuffers;
	Array<DynamicBufferToken, MAX_STORAGE_BUFFER_BINDINGS> m_storageBuffers;
	Array<DynamicBufferToken, MAX_VERTEX_ATTRIBUTES> m_vertexBuffers;
};

/// Resource group.
class ResourceGroup: public GrObject
{
public:
	/// Construct.
	ResourceGroup(GrManager* manager);

	/// Destroy.
	~ResourceGroup();

	/// Access the implementation.
	ResourceGroupImpl& getImplementation()
	{
		return *m_impl;
	}

	/// Create.
	void create(const ResourceGroupInitializer& init);

private:
	UniquePtr<ResourceGroupImpl> m_impl;
};
/// @}

} // end namespace anki

