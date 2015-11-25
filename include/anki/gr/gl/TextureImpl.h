// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/gl/GlObject.h>
#include <anki/util/DArray.h>

namespace anki {

/// @addtogroup opengl
/// @{

/// Texture container.
class TextureImpl: public GlObject
{
public:
	GLenum m_target = GL_NONE; ///< GL_TEXTURE_2D, GL_TEXTURE_3D... etc
	GLenum m_internalFormat = GL_NONE; ///< GL_COMPRESSED_RED, GL_RGB16 etc
	GLenum m_format = GL_NONE;
	GLenum m_type = GL_NONE;
	U32 m_width = 0;
	U32 m_height = 0;
	U32 m_depth = 0;
	U32 m_surfaceCount = 0;
	U8 m_mipsCount = 0;
	Bool8 m_compressed = false;
	DArray<GLuint> m_texViews; ///< Temp views for gen mips.

	TextureImpl(GrManager* manager)
		: GlObject(manager)
	{}

	~TextureImpl();

	/// Create the texture storage.
	void create(const TextureInitializer& init);

	/// Write texture data.
	void write(U32 mipmap, U32 slice, void* data, PtrSize dataSize);

	/// Generate mipmaps.
	void generateMipmaps(U surface);

	/// Copy a single slice from one texture to another.
	static void copy(const TextureImpl& src, U srcSlice, U srcLevel,
		const TextureImpl& dest, U destSlice, U destLevel);

	void bind();
};
/// @}

} // end namespace anki

