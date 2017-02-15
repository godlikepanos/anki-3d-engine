// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/common/Misc.h>
#include <anki/gr/Framebuffer.h>
#include <anki/gr/Texture.h>
#include <anki/util/StringList.h>

namespace anki
{

void logShaderErrorCode(const CString& error, const CString& source, GenericMemoryPoolAllocator<U8> alloc)
{
	StringAuto prettySrc(alloc);
	StringListAuto lines(alloc);

	static const char* padding = "==============================================================================";

	lines.splitString(source, '\n', true);

	I lineno = 0;
	for(auto it = lines.getBegin(); it != lines.getEnd(); ++it)
	{
		++lineno;
		StringAuto tmp(alloc);

		if(!it->isEmpty())
		{
			tmp.sprintf("%4d: %s\n", lineno, &(*it)[0]);
		}
		else
		{
			tmp.sprintf("%4d:\n", lineno);
		}

		prettySrc.append(tmp);
	}

	ANKI_LOGE("Shader compilation failed:\n%s\n%s\n%s\n%s\n%s\n%s",
		padding,
		&error[0],
		padding,
		&prettySrc[0],
		padding,
		&error[0]);
}

Bool textureInitInfoValid(const TextureInitInfo& inf)
{
#define ANKI_CHECK_VAL_VALIDITY(x)                                                                                     \
	do                                                                                                                 \
	{                                                                                                                  \
		if(!(x))                                                                                                       \
		{                                                                                                              \
			return false;                                                                                              \
		}                                                                                                              \
	} while(0)

	ANKI_CHECK_VAL_VALIDITY(inf.m_usage != TextureUsageBit::NONE);
	ANKI_CHECK_VAL_VALIDITY(inf.m_mipmapsCount > 0);
	ANKI_CHECK_VAL_VALIDITY(inf.m_width > 0);
	ANKI_CHECK_VAL_VALIDITY(inf.m_height > 0);
	switch(inf.m_type)
	{
	case TextureType::_2D:
		ANKI_CHECK_VAL_VALIDITY(inf.m_depth == 1);
		ANKI_CHECK_VAL_VALIDITY(inf.m_layerCount == 1);
		break;
	case TextureType::CUBE:
		ANKI_CHECK_VAL_VALIDITY(inf.m_depth == 1);
		ANKI_CHECK_VAL_VALIDITY(inf.m_layerCount == 1);
		break;
	case TextureType::_3D:
		ANKI_CHECK_VAL_VALIDITY(inf.m_depth > 0);
		ANKI_CHECK_VAL_VALIDITY(inf.m_layerCount == 1);
		break;
	case TextureType::_2D_ARRAY:
	case TextureType::CUBE_ARRAY:
		ANKI_CHECK_VAL_VALIDITY(inf.m_depth == 1);
		ANKI_CHECK_VAL_VALIDITY(inf.m_layerCount > 0);
		break;
	default:
		ANKI_CHECK_VAL_VALIDITY(0);
	};

	return true;
#undef ANKI_CHECK_VAL_VALIDITY
}

Bool framebufferInitInfoValid(const FramebufferInitInfo& inf)
{
	return inf.m_colorAttachmentCount != 0 || inf.m_depthStencilAttachment.m_texture.isCreated();
}

void getFormatInfo(const PixelFormat& fmt, U& texelComponents, U& texelBytes, U& blockSize, U& blockBytes)
{
	blockSize = 0;
	blockBytes = 0;
	texelBytes = 0;

	switch(fmt.m_components)
	{
	case ComponentFormat::R8:
		texelComponents = 1;
		texelBytes = texelComponents * 1;
		break;
	case ComponentFormat::R8G8:
		texelComponents = 2;
		texelBytes = texelComponents * 1;
		break;
	case ComponentFormat::R8G8B8:
		texelComponents = 3;
		texelBytes = texelComponents * 1;
		break;
	case ComponentFormat::R8G8B8A8:
		texelComponents = 4;
		texelBytes = texelComponents * 1;
		break;
	case ComponentFormat::R16:
		texelComponents = 1;
		texelBytes = texelComponents * 2;
		break;
	case ComponentFormat::R16G16:
		texelComponents = 2;
		texelBytes = texelComponents * 2;
		break;
	case ComponentFormat::R16G16B16:
		texelComponents = 3;
		texelBytes = texelComponents * 2;
		break;
	case ComponentFormat::R16G16B16A16:
		texelComponents = 4;
		texelBytes = texelComponents * 2;
		break;
	case ComponentFormat::R32:
		texelComponents = 1;
		texelBytes = texelComponents * 4;
		break;
	case ComponentFormat::R32G32:
		texelComponents = 2;
		texelBytes = texelComponents * 4;
		break;
	case ComponentFormat::R32G32B32:
		texelComponents = 3;
		texelBytes = texelComponents * 4;
		break;
	case ComponentFormat::R32G32B32A32:
		texelComponents = 4;
		texelBytes = texelComponents * 4;
		break;
	case ComponentFormat::R10G10B10A2:
		texelComponents = 4;
		texelBytes = 4;
		break;
	case ComponentFormat::R11G11B10:
		texelComponents = 3;
		texelBytes = 4;
		break;
	case ComponentFormat::R8G8B8_S3TC:
		texelComponents = 3;
		blockSize = 4;
		blockBytes = 8;
		break;
	case ComponentFormat::R8G8B8_ETC2:
		texelComponents = 3;
		blockSize = 4;
		blockBytes = 8;
		break;
	case ComponentFormat::R8G8B8A8_S3TC:
		texelComponents = 4;
		blockSize = 4;
		blockBytes = 16;
		break;
	case ComponentFormat::R8G8B8A8_ETC2:
		texelComponents = 4;
		blockSize = 4;
		blockBytes = 16;
		break;
	case ComponentFormat::D16:
		texelComponents = 1;
		texelBytes = texelComponents * 2;
		break;
	case ComponentFormat::D24S8:
		texelComponents = 1;
		texelBytes = texelComponents * 4;
		break;
	case ComponentFormat::D32:
		texelComponents = 1;
		texelBytes = texelComponents * 4;
		break;
	default:
		ANKI_ASSERT(0);
	}
}

PtrSize computeSurfaceSize(U width, U height, const PixelFormat& fmt)
{
	ANKI_ASSERT(width > 0 && height > 0);
	U texelComponents;
	U texelBytes;
	U blockSize;
	U blockBytes;
	getFormatInfo(fmt, texelComponents, texelBytes, blockSize, blockBytes);

	if(blockSize > 0)
	{
		// Compressed
		ANKI_ASSERT((width % blockSize) == 0);
		ANKI_ASSERT((height % blockSize) == 0);
		return (width / blockSize) * (height / blockSize) * blockBytes;
	}
	else
	{
		return width * height * texelBytes;
	}
}

PtrSize computeVolumeSize(U width, U height, U depth, const PixelFormat& fmt)
{
	ANKI_ASSERT(width > 0 && height > 0 && depth > 0);
	U texelComponents;
	U texelBytes;
	U blockSize;
	U blockBytes;
	getFormatInfo(fmt, texelComponents, texelBytes, blockSize, blockBytes);

	if(blockSize > 0)
	{
		// Compressed
		ANKI_ASSERT((width % blockSize) == 0);
		ANKI_ASSERT((height % blockSize) == 0);
		return (width / blockSize) * (height / blockSize) * blockBytes * depth;
	}
	else
	{
		return width * height * depth * texelBytes;
	}
}

} // end namespace anki
