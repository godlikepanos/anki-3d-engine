// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/gl/TextureImpl.h>
#include <anki/gr/Texture.h>
#include <anki/gr/gl/Error.h>
#include <anki/util/Functions.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/gl/GrManagerImpl.h>
#include <anki/gr/gl/TextureViewImpl.h>
#include <anki/gr/gl/RenderingThread.h>
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
	HashMap<TextureSubresourceInfo, MicroTextureView> m_views;
	GrAllocator<U8> m_alloc;

	DeleteTextureCommand(GLuint tex, HashMap<TextureSubresourceInfo, MicroTextureView>& views, GrAllocator<U8> alloc)
		: m_tex(tex)
		, m_views(std::move(views))
		, m_alloc(alloc)
	{
	}

	Error operator()(GlState& state)
	{
		// Delete views
		auto it = m_views.getBegin();
		auto end = m_views.getEnd();
		while(it != end)
		{
			const MicroTextureView& view = *it;
			glDeleteTextures(1, &view.m_glName);
			++it;
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
		static_cast<CommandBufferImpl&>(*commands).pushBackNewCommand<DeleteTextureCommand>(m_glName, m_viewsMap,
																							getAllocator());
		static_cast<CommandBufferImpl&>(*commands).flush();
	}
	else
	{
		DeleteTextureCommand cmd(m_glName, m_viewsMap, getAllocator());
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
	ANKI_ASSERT(init.isValid());

	m_width = init.m_width;
	m_height = init.m_height;
	m_depth = init.m_depth;
	m_layerCount = init.m_layerCount;
	m_target = convertTextureType(init.m_type);
	m_texType = init.m_type;
	m_format = init.m_format;
	m_usage = init.m_usage;

	convertTextureInformation(init.m_format, m_compressed, m_glFormat, m_internalFormat, m_glType, m_aspect);

	if(m_target != GL_TEXTURE_3D)
	{
		m_mipCount = min<U>(init.m_mipmapCount, computeMaxMipmapCount2d(m_width, m_height));
	}
	else
	{
		m_mipCount = min<U>(init.m_mipmapCount, computeMaxMipmapCount3d(m_width, m_height, m_depth));
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

void TextureImpl::copyFromBuffer(const TextureSubresourceInfo& subresource, GLuint pbo, PtrSize offset,
								 PtrSize dataSize) const
{
	ANKI_ASSERT(isSubresourceGoodForCopyFromBuffer(subresource));
	ANKI_ASSERT(dataSize > 0);

	const U mipmap = subresource.m_firstMipmap;
	const U w = m_width >> mipmap;
	const U h = m_height >> mipmap;
	const U d = m_depth >> mipmap;
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
	{
		const U surfIdx = computeSurfaceIdx(TextureSurfaceInfo(mipmap, 0, subresource.m_firstFace, 0));
		if(!m_compressed)
		{
			glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + surfIdx, mipmap, 0, 0, w, h, m_glFormat, m_glType,
							ptrOffset);
		}
		else
		{
			glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + surfIdx, mipmap, 0, 0, w, h, m_glFormat,
									  dataSize, ptrOffset);
		}
		break;
	}
	case GL_TEXTURE_2D_ARRAY:
	{
		const U surfIdx = computeSurfaceIdx(TextureSurfaceInfo(mipmap, 0, 0, subresource.m_firstLayer));
		if(!m_compressed)
		{
			glTexSubImage3D(m_target, mipmap, 0, 0, surfIdx, w, h, 1, m_glFormat, m_glType, ptrOffset);
		}
		else
		{
			glCompressedTexSubImage3D(m_target, mipmap, 0, 0, surfIdx, w, h, 1, m_glFormat, dataSize, ptrOffset);
		}
		break;
	}
	case GL_TEXTURE_3D:
		ANKI_ASSERT(d > 0);
		if(!m_compressed)
		{
			glTexSubImage3D(m_target, mipmap, 0, 0, 0, w, h, d, m_glFormat, m_glType, ptrOffset);
		}
		else
		{
			glCompressedTexSubImage3D(m_target, mipmap, 0, 0, 0, w, h, d, m_glFormat, dataSize, ptrOffset);
		}
		break;
	default:
		ANKI_ASSERT(0);
	}

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	ANKI_CHECK_GL_ERROR();
}

void TextureImpl::generateMipmaps2d(const TextureViewImpl& view) const
{
	ANKI_ASSERT(view.m_tex.get() == this);
	ANKI_ASSERT(isSubresourceGoodForMipmapGeneration(view.getSubresource()));
	ANKI_ASSERT(!m_compressed);

	if(m_surfaceCountPerLevel > 1)
	{
		glGenerateTextureMipmap(view.m_view.m_glName);
	}
	else
	{
		glGenerateTextureMipmap(m_glName);
	}
}

void TextureImpl::clear(const TextureSubresourceInfo& subresource, const ClearValue& clearValue) const
{
	ANKI_ASSERT(isCreated());
	ANKI_ASSERT(isSubresourceValid(subresource));
	ANKI_ASSERT(m_texType != TextureType::_3D && "TODO");

	// Find the aspect to clear
	const DepthStencilAspectBit aspect = subresource.m_depthStencilAspect;
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

	for(U mip = subresource.m_firstMipmap; mip < subresource.m_firstMipmap + subresource.m_mipmapCount; ++mip)
	{
		for(U face = subresource.m_firstFace; face < subresource.m_firstFace + subresource.m_faceCount; ++face)
		{
			for(U layer = subresource.m_firstLayer; layer < subresource.m_firstLayer + subresource.m_layerCount;
				++layer)
			{
				const U surfaceIdx = computeSurfaceIdx(TextureSurfaceInfo(mip, 0, face, layer));
				const U width = m_width >> mip;
				const U height = m_height >> mip;

				glClearTexSubImage(m_glName, mip, 0, 0, surfaceIdx, width, height, 1, format, GL_FLOAT,
								   &clearValue.m_colorf[0]);
			}
		}
	}
}

U TextureImpl::computeSurfaceIdx(const TextureSurfaceInfo& surf) const
{
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

MicroTextureView TextureImpl::getOrCreateView(const TextureSubresourceInfo& subresource) const
{
	// Quick opt: Check if the subresource refers to the whole tex
	TextureSubresourceInfo wholeTexSubresource;
	wholeTexSubresource.m_mipmapCount = getMipmapCount();
	wholeTexSubresource.m_faceCount = textureTypeIsCube(getTextureType()) ? 6 : 1;
	wholeTexSubresource.m_layerCount = getLayerCount();
	wholeTexSubresource.m_depthStencilAspect = getDepthStencilAspect();

	if(subresource == wholeTexSubresource)
	{
		MicroTextureView view{getGlName(), wholeTexSubresource.m_depthStencilAspect};
		return view;
	}

	// Continue with the regular init
	LockGuard<Mutex> lock(m_viewsMapMtx);
	auto it = m_viewsMap.find(subresource);

	if(it != m_viewsMap.getEnd())
	{
		return *it;
	}
	else
	{
		// Create a new view

		// Compute the new target if needed
		const TextureType newTexType = computeNewTexTypeOfSubresource(subresource);
		GLenum glTarget = m_target;
		if(newTexType == TextureType::_2D)
		{
			// Change that anyway
			glTarget = GL_TEXTURE_2D;
		}

		const U firstSurf = computeSurfaceIdx(
			TextureSurfaceInfo(subresource.m_firstMipmap, 0, subresource.m_firstFace, subresource.m_firstLayer));
		const U lastSurf = computeSurfaceIdx(
			TextureSurfaceInfo(subresource.m_firstMipmap, 0, subresource.m_firstFace + subresource.m_faceCount - 1,
							   subresource.m_firstLayer + subresource.m_layerCount - 1));
		ANKI_ASSERT(firstSurf <= lastSurf);

		MicroTextureView view;
		view.m_aspect = subresource.m_depthStencilAspect;

		glGenTextures(1, &view.m_glName);
		glTextureView(view.m_glName, glTarget, m_glName, m_internalFormat, subresource.m_firstMipmap,
					  subresource.m_mipmapCount, firstSurf, lastSurf - firstSurf + 1);

		m_viewsMap.emplace(getAllocator(), subresource, view);

		return view;
	}
}

} // end namespace anki
