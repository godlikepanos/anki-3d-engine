// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/Common.h>

namespace anki
{

Array<CString, U(GpuVendor::COUNT)> GPU_VENDOR_STR = {"UNKNOWN", "ARM", "NVIDIA", "AMD", "INTEL"};

static void getFormatInfo(Format fmt, U& texelComponents, U& texelBytes, U& blockSize, U& blockBytes)
{
	blockSize = 0;
	blockBytes = 0;
	texelBytes = 0;

	switch(fmt)
	{
	case Format::R8_SINT:
	case Format::R8_SNORM:
	case Format::R8_SRGB:
	case Format::R8_SSCALED:
	case Format::R8_UINT:
	case Format::R8_UNORM:
	case Format::R8_USCALED:
		texelComponents = 1;
		texelBytes = texelComponents * 1;
		break;
	case Format::R8G8_SINT:
	case Format::R8G8_SNORM:
	case Format::R8G8_SRGB:
	case Format::R8G8_SSCALED:
	case Format::R8G8_UINT:
	case Format::R8G8_UNORM:
	case Format::R8G8_USCALED:
		texelComponents = 2;
		texelBytes = texelComponents * 1;
		break;
	case Format::R8G8B8_SINT:
	case Format::R8G8B8_SNORM:
	case Format::R8G8B8_SRGB:
	case Format::R8G8B8_SSCALED:
	case Format::R8G8B8_UINT:
	case Format::R8G8B8_UNORM:
	case Format::R8G8B8_USCALED:
		texelComponents = 3;
		texelBytes = texelComponents * 1;
		break;
	case Format::R8G8B8A8_SINT:
	case Format::R8G8B8A8_SNORM:
	case Format::R8G8B8A8_SRGB:
	case Format::R8G8B8A8_SSCALED:
	case Format::R8G8B8A8_UINT:
	case Format::R8G8B8A8_UNORM:
	case Format::R8G8B8A8_USCALED:
		texelComponents = 4;
		texelBytes = texelComponents * 1;
		break;
	case Format::R16_SFLOAT:
	case Format::R16_SINT:
	case Format::R16_SNORM:
	case Format::R16_SSCALED:
	case Format::R16_UINT:
	case Format::R16_UNORM:
	case Format::R16_USCALED:
		texelComponents = 1;
		texelBytes = texelComponents * 2;
		break;
	case Format::R16G16_SFLOAT:
	case Format::R16G16_SINT:
	case Format::R16G16_SNORM:
	case Format::R16G16_SSCALED:
	case Format::R16G16_UINT:
	case Format::R16G16_UNORM:
	case Format::R16G16_USCALED:
		texelComponents = 2;
		texelBytes = texelComponents * 2;
		break;
	case Format::R16G16B16_SFLOAT:
	case Format::R16G16B16_SINT:
	case Format::R16G16B16_SNORM:
	case Format::R16G16B16_SSCALED:
	case Format::R16G16B16_UINT:
	case Format::R16G16B16_UNORM:
	case Format::R16G16B16_USCALED:
		texelComponents = 3;
		texelBytes = texelComponents * 2;
		break;
	case Format::R16G16B16A16_SFLOAT:
	case Format::R16G16B16A16_SINT:
	case Format::R16G16B16A16_SNORM:
	case Format::R16G16B16A16_SSCALED:
	case Format::R16G16B16A16_UINT:
	case Format::R16G16B16A16_UNORM:
	case Format::R16G16B16A16_USCALED:
		texelComponents = 4;
		texelBytes = texelComponents * 2;
		break;
	case Format::R32_SFLOAT:
	case Format::R32_SINT:
	case Format::R32_UINT:
		texelComponents = 1;
		texelBytes = texelComponents * 4;
		break;
	case Format::R32G32_SFLOAT:
	case Format::R32G32_SINT:
	case Format::R32G32_UINT:
		texelComponents = 2;
		texelBytes = texelComponents * 4;
		break;
	case Format::R32G32B32_SFLOAT:
	case Format::R32G32B32_SINT:
	case Format::R32G32B32_UINT:
		texelComponents = 3;
		texelBytes = texelComponents * 4;
		break;
	case Format::R32G32B32A32_SFLOAT:
	case Format::R32G32B32A32_SINT:
	case Format::R32G32B32A32_UINT:
		texelComponents = 4;
		texelBytes = texelComponents * 4;
		break;
	case Format::A2R10G10B10_UNORM_PACK32:
	case Format::A2R10G10B10_SNORM_PACK32:
	case Format::A2R10G10B10_USCALED_PACK32:
	case Format::A2R10G10B10_SSCALED_PACK32:
	case Format::A2R10G10B10_UINT_PACK32:
	case Format::A2R10G10B10_SINT_PACK32:
	case Format::A2B10G10R10_UNORM_PACK32:
	case Format::A2B10G10R10_SNORM_PACK32:
	case Format::A2B10G10R10_USCALED_PACK32:
	case Format::A2B10G10R10_SSCALED_PACK32:
	case Format::A2B10G10R10_UINT_PACK32:
	case Format::A2B10G10R10_SINT_PACK32:
		texelComponents = 4;
		texelBytes = 4;
		break;
	case Format::B10G11R11_UFLOAT_PACK32:
		texelComponents = 3;
		texelBytes = 4;
		break;
	case Format::BC1_RGB_SRGB_BLOCK:
	case Format::BC1_RGB_UNORM_BLOCK:
		texelComponents = 3;
		blockSize = 4;
		blockBytes = 8;
		break;
	case Format::BC1_RGBA_SRGB_BLOCK:
	case Format::BC1_RGBA_UNORM_BLOCK:
		texelComponents = 4;
		blockSize = 4;
		blockBytes = 8;
		break;
	case Format::BC3_SRGB_BLOCK:
	case Format::BC3_UNORM_BLOCK:
		texelComponents = 4;
		blockSize = 4;
		blockBytes = 16;
		break;
	case Format::BC6H_SFLOAT_BLOCK:
	case Format::BC6H_UFLOAT_BLOCK:
		texelComponents = 3;
		blockSize = 4;
		blockBytes = 16;
		break;
	case Format::D16_UNORM:
		texelComponents = 1;
		texelBytes = texelComponents * 2;
		break;
	case Format::D24_UNORM_S8_UINT:
		texelComponents = 1;
		texelBytes = texelComponents * 4;
		break;
	case Format::D32_SFLOAT:
		texelComponents = 1;
		texelBytes = texelComponents * 4;
		break;
	default:
		ANKI_ASSERT(0);
	}
}

PtrSize computeSurfaceSize(U width, U height, Format fmt)
{
	ANKI_ASSERT(width > 0 && height > 0);
	ANKI_ASSERT(fmt != Format::NONE);
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

PtrSize computeVolumeSize(U width, U height, U depth, Format fmt)
{
	ANKI_ASSERT(width > 0 && height > 0 && depth > 0);
	ANKI_ASSERT(fmt != Format::NONE);
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

PtrSize getFormatBytes(Format fmt)
{
	ANKI_ASSERT(fmt != Format::NONE);
	U texelComponents;
	U texelBytes;
	U blockSize;
	U blockBytes;
	getFormatInfo(fmt, texelComponents, texelBytes, blockSize, blockBytes);
	return texelBytes;
}

} // end namespace anki
