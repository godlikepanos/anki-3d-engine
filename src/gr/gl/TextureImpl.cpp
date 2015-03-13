// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/gr/gl/TextureImpl.h"
#include "anki/gr/GlError.h"
#include "anki/util/Functions.h"
#include "anki/util/DArray.h"

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

//==============================================================================
static void getTextureInformation(
	const GLenum internalFormat,
	Bool& compressed,
	GLenum& format,
	GLenum& type)
{
	switch(internalFormat)
	{
#if ANKI_GL == ANKI_GL_DESKTOP
	case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
		compressed = true;
		format = internalFormat;
		type = GL_UNSIGNED_BYTE;
		break;
	case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
		compressed = true;
		format = internalFormat;
		type = GL_UNSIGNED_BYTE;
		break;
	case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
		compressed = true;
		format = internalFormat;
		type = GL_UNSIGNED_BYTE;
		break;
	case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
		compressed = true;
		format = internalFormat;
		type = GL_UNSIGNED_BYTE;
		break;
#else
	case GL_COMPRESSED_RGB8_ETC2:
		compressed = true;
		format = internalFormat;
		type = GL_UNSIGNED_BYTE;
		break;
	case GL_COMPRESSED_RGBA8_ETC2_EAC:
		compressed = true;
		format = internalFormat;
		type = GL_UNSIGNED_BYTE;
		break;
#endif
	case GL_R8:
		compressed = false;
		format = GL_R;
		type = GL_UNSIGNED_BYTE;
		break;
	case GL_RGB8:
		compressed = false;
		format = GL_RGB;
		type = GL_UNSIGNED_BYTE;
		break;
	case GL_RGB32F:
		compressed = false;
		format = GL_RGB;
		type = GL_FLOAT;
		break;
	case GL_RGBA8:
		compressed = false;
		format = GL_RGBA;
		type = GL_UNSIGNED_BYTE;
		break;
	case GL_RG32UI:
		compressed = false;
		format = GL_RG_INTEGER;
		type = GL_UNSIGNED_INT;
		break;
	case GL_DEPTH_COMPONENT24:
		compressed = false;
		format = GL_DEPTH_COMPONENT;
		type = GL_UNSIGNED_INT;
		break;
	case GL_DEPTH_COMPONENT16:
		compressed = false;
		format = GL_DEPTH_COMPONENT;
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
Error TextureImpl::create(const Initializer& init, 
	GlAllocator<U8>& alloc)
{
	// Sanity checks
	//
	ANKI_ASSERT(!isCreated());
	ANKI_ASSERT(init.m_width > 0 && init.m_height > 0);
	ANKI_ASSERT(init.m_mipmapsCount > 0);

	// Create
	//
	glGenTextures(1, &m_glName);
	ANKI_ASSERT(m_glName != 0);

	m_width = init.m_width;
	m_height = init.m_height;
	m_depth = init.m_depth;
	m_target = init.m_target;
	m_internalFormat = init.m_internalFormat;
	m_samples = init.m_samples;
	ANKI_ASSERT(m_samples > 0);

	// Bind
	bind(0);

	// Create storage
	switch(m_target)
	{
		case GL_TEXTURE_2D:
			glTexStorage2D(
				m_target, 
				init.m_mipmapsCount, 
				m_internalFormat,
				m_width,
				m_height);
			break;
		case GL_TEXTURE_CUBE_MAP:
		case GL_TEXTURE_2D_ARRAY:
		case GL_TEXTURE_3D:
			glTexStorage3D(
				m_target,
				init.m_mipmapsCount,
				m_internalFormat,
				m_width,
				m_height,
				init.m_depth);
			break;
		case GL_TEXTURE_2D_MULTISAMPLE:
			glTexStorage2DMultisample(
				m_target,
				m_samples,
				m_internalFormat,
				m_width,
				m_height,
				GL_FALSE);
			break;
		default:
			ANKI_ASSERT(0);
	}

	// Load data
	if(init.m_data[0][0].m_ptr != nullptr)
	{
		Bool compressed = false;
		GLenum format = GL_NONE;
		GLenum type = GL_NONE;
		getTextureInformation(m_internalFormat, compressed, format, type);

		U w = m_width;
		U h = m_height;
		for(U level = 0; level < init.m_mipmapsCount; level++)
		{
			ANKI_ASSERT(init.m_data[level][0].m_ptr != nullptr);

			switch(m_target)
			{
			case GL_TEXTURE_2D:
				if(!compressed)
				{
					glTexSubImage2D(
						m_target, 
						level, 
						0,
						0,
						w,
						h, 
						format, 
						type, 
						init.m_data[level][0].m_ptr);
				}
				else
				{
					ANKI_ASSERT(init.m_data[level][0].m_ptr 
						&& init.m_data[level][0].m_size > 0);

					glCompressedTexSubImage2D(
						m_target, 
						level, 
						0,
						0,
						w, 
						h,  
						format,
						init.m_data[level][0].m_size, 
						init.m_data[level][0].m_ptr);
				}
				break;
			case GL_TEXTURE_CUBE_MAP:
				for(U face = 0; face < 6; face++)
				{
					if(!compressed)
					{
						glTexSubImage2D(
							GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 
							level, 
							0,
							0,
							w,
							h, 
							format, 
							type, 
							init.m_data[level][face].m_ptr);
					}
					else
					{

						glCompressedTexSubImage2D(
							GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 
							level, 
							0,
							0,
							w, 
							h,  
							format,
							init.m_data[level][face].m_size, 
							init.m_data[level][face].m_ptr);
					}
				}
				break;
			case GL_TEXTURE_2D_ARRAY:
			case GL_TEXTURE_3D:
				{
					ANKI_ASSERT(m_depth > 0);

					// Gather the data
					DArrayAuto<U8, GlAllocator<U8>> data(alloc);

					// Check if there are data
					if(init.m_data[level][0].m_ptr != nullptr)
					{
						PtrSize layerSize = init.m_data[level][0].m_size;
						ANKI_ASSERT(layerSize > 0);
						Error err = data.create(layerSize * m_depth);
						if(err)
						{
							return err;
						}

						for(U d = 0; d < m_depth; d++)
						{
							ANKI_ASSERT(
								init.m_data[level][d].m_size == layerSize
								&& init.m_data[level][d].m_ptr != nullptr);

							memcpy(&data[0] + d * layerSize, 
								init.m_data[level][d].m_ptr, 
								layerSize);
						}
					}

					if(!compressed)
					{
						glTexSubImage3D(
							m_target, 
							level, 
							0,
							0,
							0,
							w, 
							h, 
							m_depth,
							format, 
							type, 
							&data[0]);
					}
					else
					{							
						glCompressedTexSubImage3D(
							m_target, 
							level, 
							0,
							0,
							0,
							w, 
							h, 
							m_depth, 
							format, 
							data.getSize(), 
							&data[0]);
					}
				}
				break;
			default:
				ANKI_ASSERT(0);
			}

			w /= 2;
			h /= 2;
		}
	} // end if data

	// Set parameters
	if(m_samples == 1)
	{
		if(init.m_repeat)
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
		glTexParameteri(
			m_target, GL_TEXTURE_MAX_LEVEL, init.m_mipmapsCount - 1);

		// Set filtering type
		setFilterNoBind(init.m_filterType);

#if ANKI_GL == ANKI_GL_DESKTOP
		if(init.m_anisotropyLevel > 1)
		{
			glTexParameteri(m_target, GL_TEXTURE_MAX_ANISOTROPY_EXT, 
				GLint(init.m_anisotropyLevel));
		}
#endif
	}

	ANKI_CHECK_GL_ERROR();
	return ErrorCode::NONE;
}

//==============================================================================
void TextureImpl::destroy()
{
	if(isCreated())
	{
		glDeleteTextures(1, &m_glName);
		m_glName = 0;
	}
}

//==============================================================================
void TextureImpl::bind(U32 unit) const
{
	ANKI_ASSERT(isCreated());
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(m_target, m_glName);
}

//==============================================================================
void TextureImpl::setFilterNoBind(Filter filterType)
{
	switch(filterType)
	{
	case Filter::NEAREST:
		glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		break;
	case Filter::LINEAR:
		glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		break;
	case Filter::TRILINEAR:
		glTexParameteri(
			m_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		break;
	default:
		ANKI_ASSERT(0);
		break;
	}

	m_filter = filterType;
}

//==============================================================================
void TextureImpl::setMipmapsRange(U32 baseLevel, U32 maxLevel)
{	
	bind(0);
	glTexParameteri(m_target, GL_TEXTURE_BASE_LEVEL, baseLevel);
	glTexParameteri(m_target, GL_TEXTURE_MAX_LEVEL, maxLevel);
}

//==============================================================================
void TextureImpl::generateMipmaps()
{
	bind(0);
	glGenerateMipmap(m_target);
}

//==============================================================================
// SamplerImpl                                                                 =
//==============================================================================

//==============================================================================
void SamplerImpl::setFilter(const Filter filterType)
{
	ANKI_ASSERT(isCreated());
	switch(filterType)
	{
	case Filter::NEAREST:
		glSamplerParameteri(m_glName, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glSamplerParameteri(m_glName, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		break;
	case Filter::LINEAR:
		glSamplerParameteri(m_glName, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glSamplerParameteri(m_glName, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		break;
	case Filter::TRILINEAR:
		glSamplerParameteri(
			m_glName, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glSamplerParameteri(m_glName, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		break;
	default:
		ANKI_ASSERT(0);
		break;
	}
}

} // end namespace anki
