// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Importer/ImageImporter.h>
#include <AnKi/Util/Filesystem.h>

using namespace anki;

namespace {

class Cleanup
{
public:
	HeapAllocator<U8> m_alloc{allocAligned, nullptr};
	DynamicArrayAuto<CString> m_inputFilenames{m_alloc};
	StringAuto m_outFilename{m_alloc};
};

} // namespace

static const char* USAGE = R"(Usage: %s [options] in_files
Options:
-o <filename>          : Output filename
-t <type>              : Image type. One of: 2D, 3D, Cube, 2DArray
-no-alpha              : If the image has alpha don't store it. By default it stores it
-store-s3tc <0|1>      : Store S3TC images. Default is 1
-store-astc <0|1>      : Store ASTC images. Default is 1
-store-raw <0|1>       : Store RAW images. Default is 0
-mip-count <number>    : Max number of mipmaps. By default store until 4x4
-astc-block-size <XxY> : The size of the ASTC block size. eg 4x4. Default is 8x8
-v                     : Verbose log
-to-linear             : Convert sRGB to linear
-to-srgb               : Convert linear to sRGB
-flip-image <0|1>      : Flip the image. Default is 1
)";

static Error parseCommandLineArgs(int argc, char** argv, ImageImporterConfig& config, Cleanup& cleanup)
{
	config.m_compressions = ImageBinaryDataCompression::S3TC | ImageBinaryDataCompression::ASTC;
	config.m_noAlpha = false;
	config.m_astcBlockSize = UVec2(8u);

	// Parse config
	if(argc < 2)
	{
		// Need at least 1 input
		return Error::USER_DATA;
	}

	I i;
	for(i = 1; i < argc; i++)
	{
		CString arg = argv[i];

		if(arg == "-o")
		{
			++i;
			if(i >= argc)
			{
				return Error::USER_DATA;
			}

			cleanup.m_outFilename = argv[i];
		}
		else if(arg == "-t")
		{
			++i;
			if(i >= argc)
			{
				return Error::USER_DATA;
			}

			if(CString(argv[i]) == "2D")
			{
				config.m_type = ImageBinaryType::_2D;
			}
			else if(CString(argv[i]) == "3D")
			{
				config.m_type = ImageBinaryType::_3D;
			}
			else if(CString(argv[i]) == "Cube")
			{
				config.m_type = ImageBinaryType::CUBE;
			}
			else if(CString(argv[i]) == "2DArray")
			{
				config.m_type = ImageBinaryType::_2D_ARRAY;
			}
			else
			{
				return Error::USER_DATA;
			}
		}
		else if(CString(argv[i]) == "-no-alpha")
		{
			config.m_noAlpha = true;
		}
		else if(CString(argv[i]) == "-store-s3tc")
		{
			++i;
			if(i >= argc)
			{
				return Error::USER_DATA;
			}

			if(CString(argv[i]) == "1")
			{
				config.m_compressions |= ImageBinaryDataCompression::S3TC;
			}
			else if(CString(argv[i]) == "0")
			{
				config.m_compressions = config.m_compressions & ~ImageBinaryDataCompression::S3TC;
			}
			else
			{
				return Error::USER_DATA;
			}
		}
		else if(CString(argv[i]) == "-store-astc")
		{
			++i;
			if(i >= argc)
			{
				return Error::USER_DATA;
			}

			if(CString(argv[i]) == "1")
			{
				config.m_compressions |= ImageBinaryDataCompression::ASTC;
			}
			else if(CString(argv[i]) == "0")
			{
				config.m_compressions = config.m_compressions & ~ImageBinaryDataCompression::ASTC;
			}
			else
			{
				return Error::USER_DATA;
			}
		}
		else if(CString(argv[i]) == "-store-raw")
		{
			++i;
			if(i >= argc)
			{
				return Error::USER_DATA;
			}

			if(CString(argv[i]) == "1")
			{
				config.m_compressions |= ImageBinaryDataCompression::RAW;
			}
			else if(CString(argv[i]) == "0")
			{
				config.m_compressions = config.m_compressions & ~ImageBinaryDataCompression::RAW;
			}
			else
			{
				return Error::USER_DATA;
			}
		}
		else if(CString(argv[i]) == "-astc-block-size")
		{
			++i;
			if(i >= argc)
			{
				return Error::USER_DATA;
			}

			if(CString(argv[i]) == "4x4")
			{
				config.m_astcBlockSize = UVec2(4u);
			}
			else if(CString(argv[i]) == "8x8")
			{
				config.m_astcBlockSize = UVec2(8u);
			}
			else
			{
				return Error::USER_DATA;
			}
		}
		else if(CString(argv[i]) == "-mip-count")
		{
			++i;
			if(i >= argc)
			{
				return Error::USER_DATA;
			}

			ANKI_CHECK(CString(argv[i]).toNumber(config.m_mipmapCount));
		}
		else if(CString(argv[i]) == "-v")
		{
			LoggerSingleton::get().enableVerbosity(true);
		}
		else if(CString(argv[i]) == "-to-linear")
		{
			config.m_sRgbToLinear = true;
		}
		else if(CString(argv[i]) == "-to-srgb")
		{
			config.m_linearToSRgb = true;
		}
		else if(CString(argv[i]) == "-flip-image")
		{
			++i;
			if(i >= argc)
			{
				return Error::USER_DATA;
			}

			if(CString(argv[i]) == "1")
			{
				config.m_flipImage = true;
			}
			else if(CString(argv[i]) == "0")
			{
				config.m_flipImage = false;
			}
			else
			{
				return Error::USER_DATA;
			}
		}
		else
		{
			// Probably input, break
			break;
		}
	}

	// Inputs
	for(; i < argc; ++i)
	{
		cleanup.m_inputFilenames.emplaceBack(argv[i]);
	}

	if(cleanup.m_outFilename.getLength() == 0 || cleanup.m_inputFilenames.getSize() == 0)
	{
		return Error::USER_DATA;
	}

	config.m_inputFilenames = ConstWeakArray<CString>(cleanup.m_inputFilenames);
	config.m_outFilename = cleanup.m_outFilename;

	return Error::NONE;
}

int main(int argc, char** argv)
{
	HeapAllocator<U8> alloc(allocAligned, nullptr);

	ImageImporterConfig config;
	config.m_allocator = alloc;
	Cleanup cleanup;
	if(parseCommandLineArgs(argc, argv, config, cleanup))
	{
		ANKI_IMPORTER_LOGE(USAGE, argv[0]);
		return 1;
	}

	StringAuto tmp(alloc);
	if(getTempDirectory(tmp))
	{
		ANKI_IMPORTER_LOGE("getTempDirectory() failed");
		return 1;
	}
	config.m_tempDirectory = tmp;

	config.m_compressonatorFilename = ANKI_SOURCE_DIRECTORY "/ThirdParty/Bin/Compressonator/compressonatorcli";
	config.m_astcencFilename = ANKI_SOURCE_DIRECTORY "/ThirdParty/Bin/astcenc-avx2";

	ANKI_IMPORTER_LOGI("Image importing started: %s", config.m_outFilename.cstr());

	if(importImage(config))
	{
		ANKI_IMPORTER_LOGE("Importing failed");
		return 1;
	}

	ANKI_IMPORTER_LOGI("Image importing completed: %s", config.m_outFilename.cstr());

	return 0;
}
