// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/gl/GlObject.h>
#include <anki/gr/common/Misc.h>
#include <anki/util/DynamicArray.h>

namespace anki
{

/// @addtogroup opengl
/// @{

/// Texture container.
class TextureImpl : public GlObject
{
public:
	GLenum m_target = GL_NONE; ///< GL_TEXTURE_2D, GL_TEXTURE_3D... etc
	GLenum m_internalFormat = GL_NONE; ///< GL_COMPRESSED_RED, GL_RGB16 etc
	GLenum m_format = GL_NONE;
	GLenum m_type = GL_NONE;
	TextureType m_texType = TextureType::_1D;
	U32 m_width = 0;
	U32 m_height = 0;
	U32 m_depth = 0;
	U32 m_layerCount = 0;
	U32 m_surfaceCountPerLevel = 0;
	U8 m_mipsCount = 0;
	U8 m_faceCount = 0; ///< 6 for cubes and 1 for the rest
	Bool8 m_compressed = false;
	PixelFormat m_pformat;
	DynamicArray<GLuint> m_texViews; ///< Temp views for gen mips.
	DepthStencilAspectBit m_dsAspect = DepthStencilAspectBit::NONE;

	TextureImpl(GrManager* manager)
		: GlObject(manager)
	{
	}

	~TextureImpl();

	void checkSurfaceOrVolume(const TextureSurfaceInfo& surf) const
	{
		checkTextureSurface(m_texType, m_depth, m_mipsCount, m_layerCount, surf);
	}

	void checkSurfaceOrVolume(const TextureVolumeInfo& vol) const
	{
		ANKI_ASSERT(m_texType == TextureType::_3D);
		ANKI_ASSERT(vol.m_level < m_mipsCount);
	}

	/// Init some stuff.
	void preInit(const TextureInitInfo& init);

	/// Create the texture storage.
	void init(const TextureInitInfo& init);

	/// Write texture data.
	void writeSurface(const TextureSurfaceInfo& surf, GLuint pbo, PtrSize offset, PtrSize dataSize);

	/// Write texture data.
	void writeVolume(const TextureVolumeInfo& vol, GLuint pbo, PtrSize offset, PtrSize dataSize);

	/// Generate mipmaps.
	void generateMipmaps2d(U face, U layer);

	/// Copy a single surface from one texture to another.
	static void copy(const TextureImpl& src,
		const TextureSurfaceInfo& srcSurf,
		const TextureImpl& dest,
		const TextureSurfaceInfo& destSurf);

	void bind();

	void clear(const TextureSurfaceInfo& surf, const ClearValue& clearValue, DepthStencilAspectBit aspect);

	U computeSurfaceIdx(const TextureSurfaceInfo& surf) const;
};
/// @}

} // end namespace anki
