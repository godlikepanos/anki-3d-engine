// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/GrObject.h>
#include <anki/gr/Texture.h>
#include <anki/gr/Sampler.h>
#include <anki/gr/Buffer.h>

namespace anki
{

/// @addtogroup graphics
/// @{

/// Texture/Sampler Binding.
class TextureBinding
{
public:
	TexturePtr m_texture;
	SamplerPtr m_sampler; ///< Use it to override texture's sampler.
	TextureUsageBit m_usage = TextureUsageBit::SAMPLED_FRAGMENT;
	DepthStencilAspectMask m_aspect = DepthStencilAspectMask::DEPTH; ///< Relevant only for depth stencil textures.
};

/// Buffer binding info.
class BufferBinding
{
public:
	BufferPtr m_buffer;
	PtrSize m_offset = 0;
	PtrSize m_range = 0; ///< If zero it means the whole buffer.
	Bool m_uploadedMemory = false;
	BufferUsageBit m_usage = BufferUsageBit::NONE;
};

/// Image binding info.
class ImageBinding
{
public:
	TexturePtr m_texture;
	U8 m_level = 0;
	TextureUsageBit m_usage = TextureUsageBit::IMAGE_COMPUTE_READ_WRITE;
};

/// Resource group initializer.
class ResourceGroupInitInfo
{
public:
	Array<TextureBinding, MAX_TEXTURE_BINDINGS> m_textures;
	Array<BufferBinding, MAX_UNIFORM_BUFFER_BINDINGS> m_uniformBuffers;
	Array<BufferBinding, MAX_STORAGE_BUFFER_BINDINGS> m_storageBuffers;
	Array<ImageBinding, MAX_IMAGE_BINDINGS> m_images;
	Array<BufferBinding, MAX_VERTEX_ATTRIBUTES> m_vertexBuffers;
	BufferBinding m_indexBuffer;
	I8 m_indexSize = -1; ///< Index size in bytes. 2 or 4

	U64 computeHash() const;

private:
	void appendBufferBinding(const BufferBinding& b, U64 arr[], U& count) const;
};

/// Resource group.
class ResourceGroup : public GrObject
{
	ANKI_GR_OBJECT

anki_internal:
	UniquePtr<ResourceGroupImpl> m_impl;

	static const GrObjectType CLASS_TYPE = GrObjectType::RESOURCE_GROUP;

	/// Construct.
	ResourceGroup(GrManager* manager, U64 hash, GrObjectCache* cache);

	/// Destroy.
	~ResourceGroup();

	/// Create.
	void init(const ResourceGroupInitInfo& init);
};
/// @}

} // end namespace anki

#include <anki/gr/ResourceGroup.inl.h>
