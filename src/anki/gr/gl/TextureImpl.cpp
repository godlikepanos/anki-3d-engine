// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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
	default:
		ANKI_ASSERT(0);
	};

	return out;
}

class DeleteTextureCommand final : public GlCommand
{
public:
	GLuint m_tex;
	DynamicArray<GLuint> m_views;
	GrAllocator<U8> m_alloc;

	DeleteTextureCommand(GLuint tex, DynamicArray<GLuint>& views, GrAllocator<U8> alloc)
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

		return Error::NONE;
	}
};

TextureImpl::~TextureImpl()
{
	GrManager& manager = getManager();
	RenderingThread& thread = static_cast<GrManagerImpl&>(manager).getRenderingThread();

	if(!thread.isServerThread())
	{
		CommandBufferPtr commands;

		commands = manager.newCommandBuffer(CommandBufferInitInfo());
		static_cast<CommandBufferImpl&>(*commands).pushBackNewCommand<DeleteTextureCommand>(
			m_glName, m_texViews, getAllocator());
		static_cast<CommandBufferImpl&>(*commands).flush();
	}
	else
	{
		DeleteTextureCommand cmd(m_glName, m_texViews, getAllocator());
		cmd(static_cast<GrManagerImpl&>(manager).getState());
	}

	m_glName = 0;
}

void TextureImpl::bind() const
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(m_target, m_glName);
}

void TextureImpl::preInit(const TextureInitInfo& init)
{
	ANKI_ASSERT(textureInitInfoValid(init));

	m_width = init.m_width;
	m_height = init.m_height;
	m_depth = init.m_depth;
	m_layerCount = init.m_layerCount;
	m_target = convertTextureType(init.m_type);
	m_texType = init.m_type;
	m_format = init.m_format;

	convertTextureInformation(init.m_format, m_compressed, m_glFormat, m_internalFormat, m_glType, m_dsAspect);

	if(m_target != GL_TEXTURE_3D)
	{
		m_mipCount = min<U>(init.m_mipmapsCount, computeMaxMipmapCount2d(m_width, m_height));
	}
	else
	{
		m_mipCount = min<U>(init.m_mipmapsCount, computeMaxMipmapCount3d(m_width, m_height, m_depth));
	}

	// Surface count
	switch(m_target)
	{
	case GL_TEXTURE_1D:
	case GL_TEXTURE_2D:
	case GL_TEXTURE_2D_MULTISAMPLE:
		m_surfaceCountPerLevel = 1;
		m_faceCount = 1;
		break;
	case GL_TEXTURE_CUBE_MAP:
		m_surfaceCountPerLevel = 6;
		m_faceCount = 6;
		break;
	case GL_TEXTURE_CUBE_MAP_ARRAY:
		m_surfaceCountPerLevel = m_layerCount * 6;
		m_faceCount = 6;
		break;
	case GL_TEXTURE_2D_ARRAY:
		m_surfaceCountPerLevel = m_layerCount;
		m_faceCount = 1;
		break;
	case GL_TEXTURE_3D:
		m_surfaceCountPerLevel = m_depth;
		m_faceCount = 1;
		break;
	default:
		ANKI_ASSERT(0);
	}
}

void TextureImpl::init(const TextureInitInfo& init)
{
	ANKI_ASSERT(!isCreated());

	GrAllocator<U8> alloc = getAllocator();

	// Create
	//
	glGenTextures(1, &m_glName);
	ANKI_ASSERT(m_glName != 0);

	bind();

	// Create storage
	switch(m_target)
	{
	case GL_TEXTURE_2D:
	case GL_TEXTURE_CUBE_MAP:
		glTexStorage2D(m_target, m_mipCount, m_internalFormat, m_width, m_height);
		break;
	case GL_TEXTURE_CUBE_MAP_ARRAY:
		glTexStorage3D(m_target, m_mipCount, m_internalFormat, m_width, m_height, m_layerCount * 6);
		break;
	case GL_TEXTURE_2D_ARRAY:
		glTexStorage3D(m_target, m_mipCount, m_internalFormat, m_width, m_height, m_layerCount);
		break;
	case GL_TEXTURE_3D:
		glTexStorage3D(m_target, m_mipCount, m_internalFormat, m_width, m_height, m_depth);
		break;
	case GL_TEXTURE_2D_MULTISAMPLE:
		glTexStorage2DMultisample(m_target, init.m_samples, m_internalFormat, m_width, m_height, GL_FALSE);
		break;
	default:
		ANKI_ASSERT(0);
	}

	// Make sure that the texture is complete
	glTexParameteri(m_target, GL_TEXTURE_MAX_LEVEL, m_mipCount - 1);

	ANKI_CHECK_GL_ERROR();
}

void TextureImpl::writeSurface(const TextureSurfaceInfo& surf, GLuint pbo, PtrSize offset, PtrSize dataSize)
{
	checkSurfaceOrVolume(surf);
	ANKI_ASSERT(dataSize > 0);

	U mipmap = surf.m_level;
	U surfIdx = computeSurfaceIdx(surf);
	U w = m_width >> mipmap;
	U h = m_height >> mipmap;
	ANKI_ASSERT(w > 0);
	ANKI_ASSERT(h > 0);

	bind();
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
	const void* ptrOffset = numberToPtr<const void*>(offset);

	switch(m_target)
	{
	case GL_TEXTURE_2D:
		if(!m_compressed)
		{
			glTexSubImage2D(m_target, mipmap, 0, 0, w, h, m_glFormat, m_glType, ptrOffset);
		}
		else
		{
			glCompressedTexSubImage2D(m_target, mipmap, 0, 0, w, h, m_glFormat, dataSize, ptrOffset);
		}
		break;
	case GL_TEXTURE_CUBE_MAP:
		if(!m_compressed)
		{
			glTexSubImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + surfIdx, mipmap, 0, 0, w, h, m_glFormat, m_glType, ptrOffset);
		}
		else
		{
			glCompressedTexSubImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + surfIdx, mipmap, 0, 0, w, h, m_glFormat, dataSize, ptrOffset);
		}
		break;
	case GL_TEXTURE_2D_ARRAY:
	case GL_TEXTURE_3D:
		if(!m_compressed)
		{
			glTexSubImage3D(m_target, mipmap, 0, 0, surfIdx, w, h, 1, m_glFormat, m_glType, ptrOffset);
		}
		else
		{
			glCompressedTexSubImage3D(m_target, mipmap, 0, 0, surfIdx, w, h, 1, m_glFormat, dataSize, ptrOffset);
		}
		break;
	default:
		ANKI_ASSERT(0);
	}

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	ANKI_CHECK_GL_ERROR();
}

void TextureImpl::writeVolume(const TextureVolumeInfo& vol, GLuint pbo, PtrSize offset, PtrSize dataSize) const
{
	checkSurfaceOrVolume(vol);
	ANKI_ASSERT(dataSize > 0);
	ANKI_ASSERT(m_texType == TextureType::_3D);

	U mipmap = vol.m_level;
	U w = m_width >> mipmap;
	U h = m_height >> mipmap;
	U d = m_depth >> mipmap;
	ANKI_ASSERT(w > 0);
	ANKI_ASSERT(h > 0);
	ANKI_ASSERT(d > 0);

	bind();
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
	const void* ptrOffset = numberToPtr<const void*>(offset);

	if(!m_compressed)
	{
		glTexSubImage3D(m_target, mipmap, 0, 0, 0, w, h, d, m_glFormat, m_glType, ptrOffset);
	}
	else
	{
		glCompressedTexSubImage3D(m_target, mipmap, 0, 0, 0, w, h, d, m_glFormat, dataSize, ptrOffset);
	}

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	ANKI_CHECK_GL_ERROR();
}

void TextureImpl::generateMipmaps2d(U face, U layer)
{
	U surface = computeSurfaceIdx(TextureSurfaceInfo(0, 0, face, layer));
	ANKI_ASSERT(!m_compressed);

	if(m_surfaceCountPerLevel > 1)
	{
		// Lazily create the view array
		if(m_texViews.getSize() == 0)
		{
			m_texViews.create(getAllocator(), m_surfaceCountPerLevel);
			memset(&m_texViews[0], 0, m_texViews.getSize() * sizeof(GLuint));
		}

		// Lazily create the view
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

			glTextureView(m_texViews[surface], target, m_glName, m_internalFormat, 0, m_mipCount, surface, 1);
		}

		glGenerateTextureMipmap(m_texViews[surface]);
	}
	else
	{
		glGenerateTextureMipmap(m_glName);
	}
}

void TextureImpl::copy(const TextureImpl& src,
	const TextureSurfaceInfo& srcSurf,
	const TextureImpl& dest,
	const TextureSurfaceInfo& destSurf)
{
	ANKI_ASSERT(src.m_internalFormat == dest.m_internalFormat);
	ANKI_ASSERT(src.m_glFormat == dest.m_glFormat);
	ANKI_ASSERT(src.m_glType == dest.m_glType);

	U width = src.m_width >> srcSurf.m_level;
	U height = src.m_height >> srcSurf.m_level;

	ANKI_ASSERT(width > 0 && height > 0);
	ANKI_ASSERT(width == (dest.m_width >> destSurf.m_level) && height == (dest.m_height >> destSurf.m_level));

	U srcSurfaceIdx = src.computeSurfaceIdx(srcSurf);
	U destSurfaceIdx = dest.computeSurfaceIdx(destSurf);

	glCopyImageSubData(src.getGlName(),
		src.m_target,
		srcSurf.m_level,
		0,
		0,
		srcSurfaceIdx,
		dest.getGlName(),
		dest.m_target,
		destSurf.m_level,
		0,
		0,
		destSurfaceIdx,
		width,
		height,
		1);
}

void TextureImpl::clear(const TextureSurfaceInfo& surf, const ClearValue& clearValue, DepthStencilAspectBit aspect)
{
	ANKI_ASSERT(isCreated());
	ANKI_ASSERT(surf.m_level < m_mipCount);
	ANKI_ASSERT((aspect & m_dsAspect) == aspect);

	// Find the aspect to clear
	GLenum format;
	if(aspect == DepthStencilAspectBit::DEPTH)
	{
		ANKI_ASSERT(m_glFormat == GL_DEPTH_COMPONENT || m_glFormat == GL_DEPTH_STENCIL);
		format = GL_DEPTH_COMPONENT;
	}
	else if(aspect == DepthStencilAspectBit::STENCIL)
	{
		ANKI_ASSERT(m_glFormat == GL_STENCIL_INDEX || m_glFormat == GL_DEPTH_STENCIL);
		format = GL_STENCIL_INDEX;
	}
	else if(aspect == DepthStencilAspectBit::DEPTH_STENCIL)
	{
		ANKI_ASSERT(m_glFormat == GL_DEPTH_STENCIL);
		format = GL_DEPTH_STENCIL;
	}
	else
	{
		format = m_glFormat;
	}

	U surfaceIdx = computeSurfaceIdx(surf);
	U width = m_width >> surf.m_level;
	U height = m_height >> surf.m_level;

	glClearTexSubImage(
		m_glName, surf.m_level, 0, 0, surfaceIdx, width, height, 1, format, GL_FLOAT, &clearValue.m_colorf[0]);
}

U TextureImpl::computeSurfaceIdx(const TextureSurfaceInfo& surf) const
{
	checkSurfaceOrVolume(surf);
	U out;

	if(m_target == GL_TEXTURE_3D)
	{
		// Check depth for this level
		ANKI_ASSERT(surf.m_depth < (m_depth >> surf.m_level));
		out = surf.m_depth;
	}
	else
	{
		out = m_faceCount * surf.m_layer + surf.m_face;
	}

	ANKI_ASSERT(out < m_surfaceCountPerLevel);
	return out;
}

} // end namespace anki
