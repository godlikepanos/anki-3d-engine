// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Importer/ImageImporter.h>

#define STBI_ASSERT(x) ANKI_ASSERT(x)
#if ANKI_COMPILER_GCC_COMPATIBLE
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wfloat-conversion"
#	pragma GCC diagnostic ignored "-Wconversion"
#	pragma GCC diagnostic ignored "-Wtype-limits"
#endif
#include <Stb/stb_image.h>
#if ANKI_COMPILER_GCC_COMPATIBLE
#	pragma GCC diagnostic pop
#endif

namespace anki
{

static Bool checkConfig(const ImageImporterConfig& config)
{
#define ANKI_CFG_ASSERT(x) \
	do \
	{ \
		if(!(x)) \
		{ \
			return false; \
		} \
	} while(false)

	// Filenames
	ANKI_CFG_ASSERT(config.m_outFilename.getLength() > 0);

	for(CString in : config.m_inputFilenames)
	{
		ANKI_CFG_ASSERT(in.getLength() > 0);
	}

	// Type
	ANKI_CFG_ASSERT(config.m_type != ImageBinaryType::NONE);
	ANKI_CFG_ASSERT(config.m_inputFilenames.getSize() == 1 || config.m_type != ImageBinaryType::_2D);
	ANKI_CFG_ASSERT(config.m_inputFilenames.getSize() != 1 || config.m_type != ImageBinaryType::_2D_ARRAY);
	ANKI_CFG_ASSERT(config.m_inputFilenames.getSize() != 1 || config.m_type != ImageBinaryType::_3D);
	ANKI_CFG_ASSERT(config.m_inputFilenames.getSize() != 6 || config.m_type != ImageBinaryType::CUBE);

	// Compressions
	ANKI_CFG_ASSERT(config.m_compressions != ImageBinaryDataCompression::NONE);

	// Mip size
	ANKI_CFG_ASSERT(config.m_minMipmapDimension > 4);

#undef ANKI_CFG_ASSERT
	return true;
}

static Error importImageInternal(const ImageImporterConfig& config)
{
	if(!checkConfig(config))
	{
		ANKI_IMPORTER_LOGE("Config parameters are wrong");
		return Error::USER_DATA;
	}

	return Error::NONE;
}

Error importImage(const ImageImporterConfig& config)
{
	const Error err = importImageInternal(config);
	if(err)
	{
		ANKI_IMPORTER_LOGE("Image importing failed");
		return err;
	}

	return Error::NONE;
}

} // end namespace anki
