// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/gl/TextureImpl.h>
#include <anki/gr/Texture.h>
#include <anki/gr/gl/Error.h>
#include <anki/util/Functions.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/gl/GrManagerImpl.h>
#include <anki/gr/gl/RenderingThread.h>
#include <anki/gr/CommandBuffer.h>
#include <anki/gr/gl/CommandBufferImpl.h>

namespace anki
{

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
static void convertTextureInformation(const PixelFormat& pf,
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
		format = pf.m_srgb ? GL_COMPRESSED_SRGB_S3TC_DXT1_EXT
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
class DeleteTextureCommand final : public GlCommand
{
public:
	GLuint m_tex;
	DArray<GLuint> m_views;
	GrAllocator<U8> m_alloc;

	DeleteTextureCommand(
		GLuint tex, DArray<GLuint>& views, GrAllocator<U8> alloc)
		: m_tex(tex)
		, m_views(std::move(views))
		, m_alloc(alloc)
	{
	}

	Error operator()(GlState& state)
	{
		// Delete views
		if(m_views.getSize() > 0)
		{
			for(GLuint name : m_views)
			{
				if(name != 0)
				{
					glDeleteTextures(1, &name);
				}
			}
		}
		m_views.destroy(m_alloc);

		// Delete texture
		if(m_tex != 0)
		{
			glDeleteTextures(1, &m_tex);
		}

		return ErrorCode::NONE;
	}
};

TextureImpl::~TextureImpl()
{
	GrManager& manager = getManager();
	RenderingThread& thread = manager.getImplementation().getRenderingThread();

	if(!thread.isServerThread())
	{
		CommandBufferPtr commands;

		commands = manager.newInstance<CommandBuffer>();
		commands->getImplementation().pushBackNewCommand<DeleteTextureCommand>(
			m_glName, m_texViews, getAllocator());
		commands->flush();
	}
	else
	{
		DeleteTextureCommand cmd(m_glName, m_texViews, getAllocator());
		cmd(thread.getState());
	}

	m_glName = 0;
}

//==============================================================================
void TextureImpl::bind()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(m_target, m_glName);
}

//==============================================================================
void TextureImpl::create(const TextureInitInfo& init)
{
	GrAllocator<U8> alloc = getAllocator();
	const SamplerInitInfo& sinit = init.m_sampling;

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
	case GL_TEXTURE_CUBE_MAP:
		glTexStorage2D(
			m_target, m_mipsCount, m_internalFormat, m_width, m_height);
		break;
	case GL_TEXTURE_CUBE_MAP_ARRAY:
		glTexStorage3D(m_target,
			m_mipsCount,
			m_internalFormat,
			m_width,
			m_height,
			init.m_depth * 6);
		break;
	case GL_TEXTURE_2D_ARRAY:
	case GL_TEXTURE_3D:
		glTexStorage3D(m_target,
			m_mipsCount,
			m_internalFormat,
			m_width,
			m_height,
			init.m_depth);
		break;
	case GL_TEXTURE_2D_MULTISAMPLE:
		glTexStorage2DMultisample(m_target,
			init.m_samples,
			m_internalFormat,
			m_width,
			m_height,
			GL_FALSE);
		break;
	default:
		ANKI_ASSERT(0);
	}

	// Surface count
	switch(m_target)
	{
	case GL_TEXTURE_1D:
	case GL_TEXTURE_2D:
	case GL_TEXTURE_2D_MULTISAMPLE:
		m_surfaceCount = 1;
		break;
	case GL_TEXTURE_CUBE_MAP:
		m_surfaceCount = 6;
		break;
	case GL_TEXTURE_CUBE_MAP_ARRAY:
		m_surfaceCount = init.m_depth * 6;
		break;
	case GL_TEXTURE_2D_ARRAY:
	case GL_TEXTURE_3D:
		m_surfaceCount = init.m_depth;
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
		convertFilter(
			sinit.m_minMagFilter, sinit.m_mipmapFilter, minFilter, magFilter);

		glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, magFilter);

#if ANKI_GL == ANKI_GL_DESKTOP
		if(sinit.m_anisotropyLevel > 1)
		{
			glTexParameteri(m_target,
				GL_TEXTURE_MAX_ANISOTROPY_EXT,
				GLint(sinit.m_anisotropyLevel));
		}
#endif

		if(sinit.m_compareOperation != CompareOperation::ALWAYS)
		{
			glTexParameteri(
				m_target, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			glTexParameteri(m_target,
				GL_TEXTURE_COMPARE_FUNC,
				convertCompareOperation(sinit.m_compareOperation));
		}

		glTexParameteri(m_target, GL_TEXTURE_MIN_LOD, sinit.m_minLod);
		glTexParameteri(m_target, GL_TEXTURE_MAX_LOD, sinit.m_maxLod);
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
				m_target, mipmap, 0, 0, w, h, m_format, m_type, data);
		}
		else
		{
			glCompressedTexSubImage2D(
				m_target, mipmap, 0, 0, w, h, m_format, dataSize, data);
		}
		break;
	case GL_TEXTURE_CUBE_MAP:
		if(!m_compressed)
		{
			glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + slice,
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
			glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + slice,
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
				m_target, mipmap, 0, 0, slice, w, h, 1, m_format, m_type, data);
		}
		else
		{
			glCompressedTexSubImage3D(m_target,
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
void TextureImpl::generateMipmaps(U surface)
{
	ANKI_ASSERT(surface < m_surfaceCount);
	ANKI_ASSERT(!m_compressed);

	if(m_surfaceCount > 1)
	{
		// Create the view array
		if(m_texViews.getSize() == 0)
		{
			m_texViews.create(getAllocator(), m_surfaceCount);
			memset(&m_texViews[0], 0, m_texViews.getSize() * sizeof(GLuint));
		}

		// Create the view
		if(m_texViews[surface] == 0)
		{
			glGenTextures(1, &m_texViews[surface]);

			// Get the target
			GLenum target = GL_NONE;
			switch(m_target)
			{
			case GL_TEXTURE_CUBE_MAP:
			case GL_TEXTURE_2D_ARRAY:
			case GL_TEXTURE_CUBE_MAP_ARRAY:
				target = GL_TEXTURE_2D;
				break;
			default:
				ANKI_ASSERT(0);
			}

			glTextureView(m_texViews[surface],
				target,
				m_glName,
				m_internalFormat,
				0,
				m_mipsCount,
				surface,
				1);
		}

		glGenerateTextureMipmap(m_texViews[surface]);
	}
	else
	{
		glGenerateTextureMipmap(m_glName);
	}
}

//==============================================================================
void TextureImpl::copy(const TextureImpl& src,
	U srcSlice,
	U srcLevel,
	const TextureImpl& dest,
	U destSlice,
	U destLevel)
{
	ANKI_ASSERT(src.m_internalFormat == dest.m_internalFormat);
	ANKI_ASSERT(src.m_format == dest.m_format);
	ANKI_ASSERT(src.m_type == dest.m_type);

	U width = src.m_width >> srcLevel;
	U height = src.m_height >> srcLevel;

	ANKI_ASSERT(width > 0 && height > 0);
	ANKI_ASSERT(width == (dest.m_width >> destLevel)
		&& height == (dest.m_height >> destLevel));

	glCopyImageSubData(src.getGlName(),
		src.m_target,
		srcLevel,
		0,
		0,
		srcSlice,
		dest.getGlName(),
		dest.m_target,
		destLevel,
		0,
		0,
		destSlice,
		width,
		height,
		1);
}

} // end namespace anki
