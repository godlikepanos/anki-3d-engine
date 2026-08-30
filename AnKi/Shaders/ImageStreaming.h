// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Common.h>

ANKI_BEGIN_NAMESPACE

// An image's mipmap chain is spread across many textures. Every mipmap finer than kTailChainMipmapSize gets its own texture so that it can be
// streamed in and out on its own. All the coarser mipmaps share a single "tail chain" texture, because giving an 8x8 surface its own image wastes
// memory and descriptors.

constexpr U32 kImageDescriptorMaxTextureSize = 16 * 1024;
constexpr U32 kImageDescriptorTailChainMipmapSize = 256; // The size of the 1st mipmap of the tail chain. 256 down to kSmallestMipmapSize is 6 mipmaps
constexpr U32 kImageDescriptorSmallestMipmapSize = 8; // The chain stops here. There's no 4x4 or smaller so LODs need clamping
constexpr U32 kImageDescriptorMaxBindlessTextures = 7 + 6; // 16K image needs this amount of textures

struct ImageDescriptor
{
	U32 m_width : 16; // Size of mipmap 0 of the full image, even if that mipmap isn't resident. LOD calculations need this
	U32 m_height : 16;
	U32 m_firstMipmap : 16; // Finest resident mipmap. Sampling clamps the LOD up to this. 0 means the whole image is in memory
	U32 m_firstMipmapOfTailChain : 16; // Absolute index of the 1st mipmap that lives in the tail chain texture

	// Bindless texture index per mipmap, indexed by absolute mipmap index. Entries below m_firstMipmap are not resident. Entries from
	// m_firstMipmapOfTailChain onwards all name the same tail chain texture, so sample it with (mipmap - m_firstMipmapOfTailChain) as the LOD.
	U32 m_mipmapTextureIndices[kImageDescriptorMaxBindlessTextures];

	U32 m_padding0;
};
static_assert(sizeof(ImageDescriptor) % 16 == 0, "Needs to be 16 byte aligned since it's read from a structured buffer");

ANKI_END_NAMESPACE
