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
constexpr U32 kImageDescriptorSmallestMipmapSize = 8; // The chain stops here. There's no 4xN or Nx4 or smaller
constexpr U32 kImageDescriptorTailChainMipmapSize = 256; // The size of the 1st mipmap of the tail chain. 256 down to kSmallestMipmapSize is 6 mipmaps
constexpr U32 kImageDescriptorTailChainMipmapCount = 6;
constexpr U32 kImageDescriptorMaxMipmaps = 12; // Mips from kImageDescriptorMaxTextureSize to kImageDescriptorSmallestMipmapSize
constexpr U32 kImageDescriptorMaxBindlessTextures = 6 + 1; // Textures from to fit kImageDescriptorMaxTextureSize and the tail chain texture

struct ImageDescriptor
{
	U32 m_width : 16; // Size of mipmap 0 of the full image, even if that mipmap isn't resident. LOD calculations need this
	U32 m_height : 16;
	U32 m_depthOrLayerCount : 16;
	U32 m_firstMipmap : 8; // Finest resident mipmap. Points to m_bindlessTextureIndexAndLod. 0 means the whole image is in memory
	U32 m_lastMipmap : 8; // Last mipmap. Points to m_bindlessTextureIndexAndLod

	// Every U32 packs the index to the bindless texture (24bit) and the mipmap to that texture (8bit). Used as m_bindlessTextureIndexAndLod[mipmap]
	U32 m_bindlessTextureIndexAndLod[kImageDescriptorMaxMipmaps];

	U32 m_padding[2];
};
static_assert(sizeof(ImageDescriptor) % 16 == 0);

ANKI_END_NAMESPACE
