// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/gl/TextureImpl.h"
#include "anki/gr/Texture.h"
#include "anki/gr/gl/Error.h"
#include "anki/util/Functions.h"

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

//==============================================================================
static GLenum convertTextureType(TextureType type)
{
	GLenum out = GL_NONE;
	switch(type)
	{
	case TextureType::_1D:
		out = GL_TEXTURE_1D;
		break;
	case TextureType::_2D:
		out = GL_TEXTURE_2D;
		break;
	case TextureType::_3D:
		out = GL_TEXTURE_3D;
		break;
	case TextureType::_2D_ARRAY:
		out = GL_TEXTURE_2D_ARRAY;
		break;
	case TextureType::CUBE:
		out = GL_TEXTURE_CUBE_MAP;
		break;
	case TextureType::CUBE_ARRAY:
		out = GL_TEXTURE_CUBE_MAP_ARRAY;
		break;
	};

	return out;
}

//==============================================================================
static void convertTextureInformation(
	const PixelFormat& pf,
	Bool8& compressed,
	GLenum& format,
	GLenum& internalFormat,
	GLenum& type)
{
	compressed = pf.m_components >= ComponentFormat::FIRST_COMPRESSED
		&& pf.m_components <= ComponentFormat::LAST_COMPRESSED;

	switch(pf.m_components)
	{
#if ANKI_GL == ANKI_GL_DESKTOP
	case ComponentFormat::R8G8B8_S3TC:
		format = pf.m_srgb
			? GL_COMPRESSED_SRGB_S3TC_DXT1_EXT
			: GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
		internalFormat = format;
		type = GL_UNSIGNED_BYTE;
		break;
	case ComponentFormat::R8G8B8A8_S3TC:
		format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		internalFormat = format;
		type = GL_UNSIGNED_BYTE;
		break;
#else
	case ComponentFormat::R8G8B8_ETC2:
		format = GL_COMPRESSED_RGB8_ETC2;
		internalFormat = format;
		type = GL_UNSIGNED_BYTE;
		break;
	case ComponentFormat::R8G8B8A8_ETC2:
		format = GL_COMPRESSED_RGBA8_ETC2_EAC;
		internalFormat = format;
		type = GL_UNSIGNED_BYTE;
		break;
#endif
	case ComponentFormat::R8:
		format = GL_R;
		internalFormat = GL_R8;
		type = GL_UNSIGNED_BYTE;
		break;
	case ComponentFormat::R8G8B8:
		format = GL_RGB;
		internalFormat = GL_RGB8;
		type = GL_UNSIGNED_BYTE;
		break;
	case ComponentFormat::R8G8B8A8:
		format = GL_RGBA;
		internalFormat = GL_RGBA8;
		type = GL_UNSIGNED_BYTE;
		break;
	case ComponentFormat::R32G32:
		if(pf.m_transform == TransformFormat::FLOAT)
		{
			format = GL_RG;
			internalFormat = GL_RG32F;
			type = GL_FLOAT;
		}
		else if(pf.m_transform == TransformFormat::UINT)
		{
			format = GL_RG_INTEGER;
			internalFormat = GL_RG32UI;
			type = GL_UNSIGNED_INT;
		}
		else
		{
			ANKI_ASSERT(0 && "TODO");
		}
		break;
	case ComponentFormat::R32G32B32:
		if(pf.m_transform == TransformFormat::FLOAT)
		{
			format = GL_RGB;
			internalFormat = GL_RGB32F;
			type = GL_FLOAT;
		}
		else if(pf.m_transform == TransformFormat::UINT)
		{
			format = GL_RGB_INTEGER;
			internalFormat = GL_RGB32UI;
			type = GL_UNSIGNED_INT;
		}
		else
		{
			ANKI_ASSERT(0 && "TODO");
		}
		break;
	case ComponentFormat::R16G16B16:
		if(pf.m_transform == TransformFormat::FLOAT)
		{
			format = GL_RGB;
			internalFormat = GL_RGB16F;
			type = GL_FLOAT;
		}
		else if(pf.m_transform == TransformFormat::UINT)
		{
			format = GL_RGB_INTEGER;
			internalFormat = GL_RGB16UI;
			type = GL_UNSIGNED_INT;
		}
		else
		{
			ANKI_ASSERT(0 && "TODO");
		}
		break;
	case ComponentFormat::R11G11B10:
		if(pf.m_transform == TransformFormat::FLOAT)
		{
			format = GL_RGB;
			internalFormat = GL_R11F_G11F_B10F;
			type = GL_FLOAT;
		}
		else
		{
			ANKI_ASSERT(0 && "TODO");
		}
		break;
	case ComponentFormat::R10G10B10A2:
		if(pf.m_transform == TransformFormat::UNORM)
		{
			format = GL_RGBA;
			internalFormat = GL_RGB10_A2;
			type = GL_UNSIGNED_INT;
		}
		else
		{
			ANKI_ASSERT(0 && "TODO");
		}
		break;
	case ComponentFormat::D24:
		format = GL_DEPTH_COMPONENT;
		internalFormat = GL_DEPTH_COMPONENT24;
		type = GL_UNSIGNED_INT;
		break;
	case ComponentFormat::D16:
		format = GL_DEPTH_COMPONENT;
		internalFormat = GL_DEPTH_COMPONENT16;
		type = GL_UNSIGNED_SHORT;
		break;
	default:
		ANKI_ASSERT(0);
	}
}

//==============================================================================
// TextureImpl                                                                 =
//==============================================================================

//==============================================================================
void TextureImpl::bind()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(m_target, m_glName);
}

//==============================================================================
void TextureImpl::create(const TextureInitializer& init)
{
	GrAllocator<U8> alloc = getAllocator();
	const SamplerInitializer& sinit = init.m_sampling;

	// Sanity checks
	//
	ANKI_ASSERT(!isCreated());
	ANKI_ASSERT(init.m_width > 0 && init.m_height > 0);
	ANKI_ASSERT(init.m_mipmapsCount > 0);
	ANKI_ASSERT(init.m_samples > 0);

	// Create
	//
	glGenTextures(1, &m_glName);
	ANKI_ASSERT(m_glName != 0);

	m_width = init.m_width;
	m_height = init.m_height;
	m_depth = init.m_depth;
	m_target = convertTextureType(init.m_type);

	convertTextureInformation(
		init.m_format, m_compressed, m_format, m_internalFormat, m_type);

	m_mipsCount =
		min<U>(init.m_mipmapsCount, computeMaxMipmapCount(m_width, m_height));

	// Bind
	bind();

	// Create storage
	switch(m_target)
	{
	case GL_TEXTURE_2D:
		glTexStorage2D(
			m_target,
			m_mipsCount,
			m_internalFormat,
			m_width,
			m_height);
		break;
	case GL_TEXTURE_CUBE_MAP_ARRAY:
		glTexStorage3D(
			m_target,
			m_mipsCount,
			m_internalFormat,
			m_width,
			m_height,
			init.m_depth * 6);
		break;
	case GL_TEXTURE_CUBE_MAP:
	case GL_TEXTURE_2D_ARRAY:
	case GL_TEXTURE_3D:
		glTexStorage3D(
			m_target,
			m_mipsCount,
			m_internalFormat,
			m_width,
			m_height,
			init.m_depth);
		break;
	case GL_TEXTURE_2D_MULTISAMPLE:
		glTexStorage2DMultisample(
			m_target,
			init.m_samples,
			m_internalFormat,
			m_width,
			m_height,
			GL_FALSE);
		break;
	default:
		ANKI_ASSERT(0);
	}

	// Set parameters
	if(init.m_samples == 1)
	{
		if(sinit.m_repeat)
		{
			glTexParameteri(m_target, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(m_target, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
		else
		{
			glTexParameteri(m_target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(m_target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}

		// Make sure that the texture is complete
		glTexParameteri(m_target, GL_TEXTURE_MAX_LEVEL, m_mipsCount - 1);

		// Set filtering type
		GLenum minFilter = GL_NONE;
		GLenum magFilter = GL_NONE;
		convertFilter(sinit.m_minMagFilter, sinit.m_mipmapFilter,
			minFilter, magFilter);

		glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, magFilter);

#if ANKI_GL == ANKI_GL_DESKTOP
		if(sinit.m_anisotropyLevel > 1)
		{
			glTexParameteri(m_target, GL_TEXTURE_MAX_ANISOTROPY_EXT,
				GLint(sinit.m_anisotropyLevel));
		}
#endif

		if(sinit.m_compareOperation != CompareOperation::ALWAYS)
		{
			glTexParameteri(
				m_target, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			glTexParameteri(m_target, GL_TEXTURE_COMPARE_FUNC,
				convertCompareOperation(sinit.m_compareOperation));
		}
	}

	ANKI_CHECK_GL_ERROR();
}

//==============================================================================
void TextureImpl::write(U32 mipmap, U32 slice, void* data, PtrSize dataSize)
{
	ANKI_ASSERT(data);
	ANKI_ASSERT(dataSize > 0);
	ANKI_ASSERT(mipmap < m_mipsCount);

	U w = m_width >> mipmap;
	U h = m_height >> mipmap;

	ANKI_ASSERT(w > 0);
	ANKI_ASSERT(h > 0);

	bind();

	switch(m_target)
	{
	case GL_TEXTURE_2D:
		if(!m_compressed)
		{
			glTexSubImage2D(
				m_target,
				mipmap,
				0,
				0,
				w,
				h,
				m_format,
				m_type,
				data);
		}
		else
		{
			glCompressedTexSubImage2D(
				m_target,
				mipmap,
				0,
				0,
				w,
				h,
				m_format,
				dataSize,
				data);
		}
		break;
	case GL_TEXTURE_CUBE_MAP:
		if(!m_compressed)
		{
			glTexSubImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + slice,
				mipmap,
				0,
				0,
				w,
				h,
				m_format,
				m_type,
				data);
		}
		else
		{
			glCompressedTexSubImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + slice,
				mipmap,
				0,
				0,
				w,
				h,
				m_format,
				dataSize,
				data);
		}
		break;
	case GL_TEXTURE_2D_ARRAY:
	case GL_TEXTURE_3D:
		ANKI_ASSERT(m_depth > 0);
		ANKI_ASSERT(slice < m_depth);

		if(!m_compressed)
		{
			glTexSubImage3D(
				m_target,
				mipmap,
				0,
				0,
				slice,
				w,
				h,
				1,
				m_format,
				m_type,
				data);
		}
		else
		{
			glCompressedTexSubImage3D(
				m_target,
				mipmap,
				0,
				0,
				slice,
				w,
				h,
				1,
				m_format,
				dataSize,
				data);
		}
		break;
	default:
		ANKI_ASSERT(0);
	}

	ANKI_CHECK_GL_ERROR();
}

//==============================================================================
void TextureImpl::generateMipmaps()
{
	ANKI_ASSERT(!m_compressed);
	bind();
	glGenerateMipmap(m_target);
}

//==============================================================================
U32 TextureImpl::computeMaxMipmapCount(U32 w, U32 h)
{
	U32 s = min(w, h);
	U count = 0;
	while(s)
	{
		s /= 2;
		++count;
	}

	return count;
}

} // end namespace anki
