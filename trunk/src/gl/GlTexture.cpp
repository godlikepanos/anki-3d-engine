#include "anki/gl/GlTexture.h"
#include "anki/gl/GlError.h"
#include "anki/util/Functions.h"
#include "anki/util/Vector.h"

namespace anki {

//==============================================================================
// Misc                                                                        =
//==============================================================================

namespace {

//==============================================================================
Bool isCompressedInternalFormat(const GLenum internalFormat)
{
	Bool out = false;
	switch(internalFormat)
	{
#if ANKI_GL == ANKI_GL_DESKTOP
	case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
		out = true;
		break;
#else
	case GL_COMPRESSED_RGB8_ETC2:
	case GL_COMPRESSED_RGBA8_ETC2_EAC:
		out = true;
		break;
#endif
	default:
		out = false;
	}
	return out;
}

} // end anonymous namespace


//==============================================================================
// GlTexture                                                                   =
//==============================================================================

//==============================================================================
GlTexture& GlTexture::operator=(GlTexture&& b)
{
	destroy();
	Base::operator=(std::forward<Base>(b));

	m_target = b.m_target;
	m_internalFormat = b.m_internalFormat; 
	m_format = b.m_format;
	m_type = b.m_type;
	m_width = b.m_width;
	m_height = b.m_height;
	m_depth = b.m_depth;
	m_filter = b.m_filter;
	m_samples = b.m_samples;
	return *this;
}

//==============================================================================
void GlTexture::create(const Initializer& init, 
	GlGlobalHeapAllocator<U8>& alloc)
{
	// Sanity checks
	//
	ANKI_ASSERT(!isCreated());
	ANKI_ASSERT(init.m_internalFormat > 4 && "Deprecated internal format");
	ANKI_ASSERT(init.m_width > 0 && init.m_height > 0);

	// Create
	//
	glGenTextures(1, &m_glName);
	ANKI_ASSERT(m_glName != 0);

	m_width = init.m_width;
	m_height = init.m_height;
	m_depth = init.m_depth;
	m_target = init.m_target;
	m_internalFormat = init.m_internalFormat;
	m_format = init.m_format;
	m_type = init.m_type;
	m_samples = init.m_samples;
	ANKI_ASSERT(m_samples > 0);

	// Bind
	bind(0);

	// Load data
	U w = m_width;
	U h = m_height;
	ANKI_ASSERT(init.m_mipmapsCount > 0);
	for(U level = 0; level < init.m_mipmapsCount; level++)
	{
		switch(m_target)
		{
		case GL_TEXTURE_2D:
			if(!isCompressedInternalFormat(m_internalFormat))
			{
				glTexImage2D(
					m_target, 
					level, 
					m_internalFormat, 
					w,
					h, 
					0, 
					m_format, 
					m_type, 
					init.m_data[level][0].m_ptr);
			}
			else
			{
				ANKI_ASSERT(init.m_data[level][0].m_ptr 
					&& init.m_data[level][0].m_size > 0);

				glCompressedTexImage2D(
					m_target, 
					level, 
					m_internalFormat,
					w, 
					h, 
					0, 
					init.m_data[level][0].m_size, 
					init.m_data[level][0].m_ptr);
			}
			break;
		case GL_TEXTURE_CUBE_MAP:
			for(U face = 0; face < 6; face++)
			{
				if(!isCompressedInternalFormat(m_internalFormat))
				{
					glTexImage2D(
						GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 
						level, 
						m_internalFormat, 
						w, 
						h, 
						0, 
						m_format, 
						m_type, 
						init.m_data[level][face].m_ptr);
				}
				else
				{
					glCompressedTexImage2D(
						GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 
						level, 
						m_internalFormat,
						w, 
						h, 
						0, 
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
				Vector<U8, GlGlobalHeapAllocator<U8>> data(alloc);

				// Check if there are data
				if(init.m_data[level][0].m_ptr != nullptr)
				{
					PtrSize layerSize = init.m_data[level][0].m_size;
					ANKI_ASSERT(layerSize > 0);
					data.resize(layerSize * m_depth);

					for(U d = 0; d < m_depth; d++)
					{
						ANKI_ASSERT(init.m_data[level][d].m_size == layerSize
							&& init.m_data[level][d].m_ptr != nullptr);

						memcpy(&data[0] + d * layerSize, 
							init.m_data[level][d].m_ptr, 
							layerSize);
					}
				}

				if(!isCompressedInternalFormat(m_internalFormat))
				{
					glTexImage3D(
						m_target, 
						level, 
						m_internalFormat, 
						w, 
						h, 
						m_depth,
						0, 
						m_format, 
						m_type, 
						(data.size() > 0) ? &data[0] : nullptr);
				}
				else
				{
					ANKI_ASSERT(data.size() > 0);
						
					glCompressedTexImage3D(
						m_target, 
						level, 
						m_internalFormat,
						w, 
						h, 
						m_depth, 
						0, 
						data.size(), 
						(data.size() > 0) ? &data[0] : nullptr);
				}
			}
			break;
#if ANKI_GL == ANKI_GL_DESKTOP
		case GL_TEXTURE_2D_MULTISAMPLE:
			glTexImage2DMultisample(
				m_target,
				m_samples,
				m_internalFormat,
				w,
				h,
				GL_FALSE);
			break;
#endif
		default:
			ANKI_ASSERT(0);
		}

		w /= 2;
		h /= 2;
	}

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

		if(init.m_genMipmaps)
		{
			glGenerateMipmap(m_target);
		}
		else
		{
			// Make sure that the texture is complete
			glTexParameteri(
				m_target, GL_TEXTURE_MAX_LEVEL, init.m_mipmapsCount - 1);
		}

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
}

//==============================================================================
void GlTexture::destroy()
{
	if(isCreated())
	{
		glDeleteTextures(1, &m_glName);
		m_glName = 0;
	}
}

//==============================================================================
void GlTexture::bind(U32 unit) const
{
	ANKI_ASSERT(isCreated());
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(m_target, m_glName);
}

//==============================================================================
void GlTexture::setFilterNoBind(Filter filterType)
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
void GlTexture::setMipmapsRange(U32 baseLevel, U32 maxLevel)
{	
	bind(0);
	glTexParameteri(m_target, GL_TEXTURE_BASE_LEVEL, baseLevel);
	glTexParameteri(m_target, GL_TEXTURE_MAX_LEVEL, maxLevel);
}

//==============================================================================
void GlTexture::generateMipmaps()
{
	bind(0);
	glGenerateMipmap(m_target);
}

//==============================================================================
// GlSampler                                                                   =
//==============================================================================

//==============================================================================
void GlSampler::setFilter(const Filter filterType)
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
