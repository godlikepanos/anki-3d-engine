// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/StreamingImageResource.h>
#include <AnKi/Resource/ImageLoader.h>
#include <AnKi/Resource/AsyncLoader.h>
#include <AnKi/Util/Filesystem.h>
#include <AnKi/GpuMemory/CopyEngine.h>

namespace anki {

static Format computeFormat(const ImageLoader& loader)
{
	Format fmt = Format::kNone;

	if(loader.getColorFormat() == ImageBinaryColorFormat::kRgb8)
	{
		switch(loader.getCompression())
		{
		case ImageBinaryDataCompression::kRaw:
			fmt = Format::kR8G8B8_Unorm;
			break;
		case ImageBinaryDataCompression::kS3tc:
			fmt = Format::kBC1_Rgba_Unorm_Block;
			break;
		case ImageBinaryDataCompression::kAstc:
			if(loader.getAstcBlockSize() == UVec2(4u))
			{
				fmt = Format::kASTC_4x4_Unorm_Block;
			}
			else
			{
				ANKI_ASSERT(loader.getAstcBlockSize() == UVec2(8u));
				fmt = Format::kASTC_8x8_Unorm_Block;
			}
			break;
		default:
			ANKI_ASSERT(0);
		}
	}
	else if(loader.getColorFormat() == ImageBinaryColorFormat::kSrgb8)
	{
		switch(loader.getCompression())
		{
		case ImageBinaryDataCompression::kRaw:
			fmt = Format::kR8G8B8_Srgb;
			break;
		case ImageBinaryDataCompression::kS3tc:
			fmt = Format::kBC1_Rgba_Srgb_Block;
			break;
		case ImageBinaryDataCompression::kAstc:
			if(loader.getAstcBlockSize() == UVec2(4u))
			{
				fmt = Format::kASTC_4x4_Srgb_Block;
			}
			else
			{
				ANKI_ASSERT(loader.getAstcBlockSize() == UVec2(8u));
				fmt = Format::kASTC_8x8_Srgb_Block;
			}
			break;
		default:
			ANKI_ASSERT(0);
		}
	}
	else if(loader.getColorFormat() == ImageBinaryColorFormat::kRgba8)
	{
		switch(loader.getCompression())
		{
		case ImageBinaryDataCompression::kRaw:
			fmt = Format::kR8G8B8A8_Unorm;
			break;
		case ImageBinaryDataCompression::kS3tc:
			fmt = Format::kBC3_Unorm_Block;
			break;
		case ImageBinaryDataCompression::kAstc:
			if(loader.getAstcBlockSize() == UVec2(4u))
			{
				fmt = Format::kASTC_4x4_Unorm_Block;
			}
			else
			{
				ANKI_ASSERT(loader.getAstcBlockSize() == UVec2(8u));
				fmt = Format::kASTC_8x8_Unorm_Block;
			}
			break;
		default:
			ANKI_ASSERT(0);
		}
	}
	else if(loader.getColorFormat() == ImageBinaryColorFormat::kSrgba8)
	{
		switch(loader.getCompression())
		{
		case ImageBinaryDataCompression::kRaw:
			fmt = Format::kR8G8B8A8_Srgb;
			break;
		case ImageBinaryDataCompression::kS3tc:
			fmt = Format::kBC3_Srgb_Block;
			break;
		case ImageBinaryDataCompression::kAstc:
			if(loader.getAstcBlockSize() == UVec2(4u))
			{
				fmt = Format::kASTC_4x4_Srgb_Block;
			}
			else
			{
				ANKI_ASSERT(loader.getAstcBlockSize() == UVec2(8u));
				fmt = Format::kASTC_8x8_Srgb_Block;
			}
			break;
		default:
			ANKI_ASSERT(0);
		}
	}
	else if(loader.getColorFormat() == ImageBinaryColorFormat::kRgbFloat)
	{
		switch(loader.getCompression())
		{
		case ImageBinaryDataCompression::kS3tc:
			fmt = Format::kBC6H_Ufloat_Block;
			break;
		case ImageBinaryDataCompression::kAstc:
			ANKI_ASSERT(loader.getAstcBlockSize() == UVec2(8u));
			fmt = Format::kASTC_8x8_Sfloat_Block;
			break;
		default:
			ANKI_ASSERT(0);
		}
	}
	else if(loader.getColorFormat() == ImageBinaryColorFormat::kRgbaFloat)
	{
		switch(loader.getCompression())
		{
		case ImageBinaryDataCompression::kRaw:
			fmt = Format::kR32G32B32A32_Sfloat;
			break;
		case ImageBinaryDataCompression::kAstc:
			ANKI_ASSERT(loader.getAstcBlockSize() == UVec2(8u));
			fmt = Format::kASTC_8x8_Sfloat_Block;
			break;
		default:
			ANKI_ASSERT(0);
		}
	}
	else
	{
		ANKI_ASSERT(0);
	}

	return fmt;
}

Error StreamingImageResourceManager::init()
{
	m_imageDescriptorsBuff = GpuSceneBuffer::getSingleton().allocateStructuredBuffer<ImageDescriptor>(g_cvarRsrcMaxImageDescriptors);

	for(U32 i = 0; i < g_cvarRsrcMaxImageDescriptors; ++i)
	{
		m_freeDecriptorMask.setBit(i);
	}

	return Error::kNone;
}

U32 StreamingImageResourceManager::newImageDescriptor()
{
	const U32 index = m_freeDecriptorMask.getMostSignificantBit();
	if(index == kMaxU32)
	{
		ANKI_RESOURCE_LOGF("Out of free descriptors. Increase %s", g_cvarRsrcMaxImageDescriptors.getName().cstr());
	}

	m_freeDecriptorMask.unsetBit(index);

	return index;
}

void StreamingImageResourceManager::freeImageDescriptor(U32 index)
{
	ANKI_ASSERT(!m_freeDecriptorMask.getBit(index));
	m_freeDecriptorMask.setBit(index);
}

void StreamingImageResourceManager::uploadImageDescriptor(U32 index, const ImageDescriptor& desc)
{
	ANKI_ASSERT(!m_freeDecriptorMask.getBit(index));
	const PtrSize offset = m_imageDescriptorsBuff.getOffset() + index * sizeof(ImageDescriptor);
	GpuSceneMicroPatcher::getSingleton().newCopy(offset, desc);
}

class StreamingImageResource::LoadingContext
{
public:
	ImageLoader m_loader{&ResourceMemoryPool::getSingleton()};
	StreamingImageResourcePtr m_image;
};

// Image upload async task.
class StreamingImageResource::TexUploadTask : public AsyncLoaderTask
{
public:
	StreamingImageResource::LoadingContext m_ctx;

	Error operator()([[maybe_unused]] AsyncLoaderTaskContext& ctx) final
	{
		return m_ctx.m_image->loadAsync(m_ctx);
	}
};

StreamingImageResource::~StreamingImageResource()
{
	if(m_imageDescriptorIdx != kMaxU32)
	{
		StreamingImageResourceManager::getSingleton().freeImageDescriptor(m_imageDescriptorIdx);
	}
}

Error StreamingImageResource::load(const ResourceFilename& filename, Bool async)
{
	return Error::kNone;
}

} // end namespace anki
