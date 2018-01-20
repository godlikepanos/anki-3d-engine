// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/Common.h>

namespace anki
{

Array<CString, U(GpuVendor::COUNT)> GPU_VENDOR_STR = {{"UNKNOWN", "ARM", "NVIDIA", "AMD", "INTEL"}};

static void getFormatInfo(const PixelFormat& fmt, U& texelComponents, U& texelBytes, U& blockSize, U& blockBytes)
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
	case ComponentFormat::R8G8B8_BC1:
		texelComponents = 3;
		blockSize = 4;
		blockBytes = 8;
		break;
	case ComponentFormat::R8G8B8A8_BC3:
		texelComponents = 4;
		blockSize = 4;
		blockBytes = 16;
		break;
	case ComponentFormat::R16G16B16_BC6:
		texelComponents = 3;
		blockSize = 4;
		blockBytes = 16;
		break;
	case ComponentFormat::R8G8B8_ETC2:
		texelComponents = 3;
		blockSize = 4;
		blockBytes = 8;
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
	ANKI_ASSERT(fmt.isValid());
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
	ANKI_ASSERT(fmt.isValid());
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
