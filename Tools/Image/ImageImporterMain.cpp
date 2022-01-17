// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Importer/ImageImporter.h>
#include <AnKi/Util/Filesystem.h>

using namespace anki;

static const char* USAGE = R"(Usage: %s in_files out_file [options]
Options:
-t <type>              : Image type. One of: 2D, 3D, Cube, 2DArray
-no-alpha              : If the image has alpha don't store it. By default it stores it
-store-s3tc <0|1>      : Store S3TC images. Default is 1
-store-astc <0|1>      : Store ASTC images. Default is 1
-store-raw <0|1>       : Store RAW images. Default is 0
-mip-count <number>    : Max number of mipmaps. By default store until 4x4
-astc-block-size <XxY> : The size of the ASTC block size. eg 4x4. Default is 8x8
-verbose               : Verbose log
-to-linear             : Convert sRGB to linear
-to-srgb               : Convert linear to sRGB
)";

static Error parseCommandLineArgs(int argc, char** argv, ImageImporterConfig& config,
								  DynamicArrayAuto<StringAuto>& filenames, DynamicArrayAuto<CString>& cfilenames)
{
	config.m_compressions = ImageBinaryDataCompression::S3TC | ImageBinaryDataCompression::ASTC;
	config.m_noAlpha = false;
	config.m_astcBlockSize = UVec2(8u);

	// Parse config
	if(argc < 3)
	{
		return Error::USER_DATA;
	}

	for(I i = 1; i < argc; i++)
	{
		if(CString(argv[i]) == "-t")
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
		else if(CString(argv[i]) == "-verbose")
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
		else
		{
			filenames.emplaceBack(filenames.getAllocator(), argv[i]);
		}
	}

	if(filenames.getSize() < 2)
	{
		return Error::USER_DATA;
	}

	cfilenames.create(filenames.getSize());
	for(U32 i = 0; i < filenames.getSize(); ++i)
	{
		cfilenames[i] = filenames[i];
	}

	config.m_inputFilenames = ConstWeakArray<CString>(&cfilenames[0], cfilenames.getSize() - 1);
	config.m_outFilename = cfilenames.getBack();

	return Error::NONE;
}

int main(int argc, char** argv)
{
	HeapAllocator<U8> alloc(allocAligned, nullptr);

	ImageImporterConfig config;
	config.m_allocator = alloc;
	DynamicArrayAuto<StringAuto> filenames(alloc);
	DynamicArrayAuto<CString> cfilenames(alloc);
	if(parseCommandLineArgs(argc, argv, config, filenames, cfilenames))
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

	StringAuto p(alloc);
	getParentFilepath(argv[0], p);
	StringAuto compressonatorPath(alloc);
	compressonatorPath.sprintf("%s/../../ThirdParty/Bin/Compressonator:%s", p.cstr(), getenv("PATH"));
	config.m_compressonatorPath = compressonatorPath;

	StringAuto astcencPath(alloc);
	astcencPath.sprintf("%s/../../ThirdParty/Bin:%s", p.cstr(), getenv("PATH"));
	config.m_astcencPath = astcencPath;

	ANKI_IMPORTER_LOGI("Image importing started: %s", config.m_outFilename.cstr());

	if(importImage(config))
	{
		ANKI_IMPORTER_LOGE("Importing failed");
		return 1;
	}

	ANKI_IMPORTER_LOGI("Image importing completed: %s", config.m_outFilename.cstr());

	return 0;
}
