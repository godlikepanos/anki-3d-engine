// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Importer/ImageImporter.h>
#include <AnKi/Importer/TinyExr.h>
#include <AnKi/Gr/Common.h>
#include <AnKi/Resource/Stb.h>
#include <AnKi/Util/Process.h>
#include <AnKi/Util/File.h>
#include <AnKi/Util/Filesystem.h>

namespace anki {

namespace {

class SurfaceOrVolumeData
{
public:
	DynamicArrayRaii<U8, PtrSize> m_pixels;
	DynamicArrayRaii<U8, PtrSize> m_s3tcPixels;
	DynamicArrayRaii<U8, PtrSize> m_astcPixels;

	SurfaceOrVolumeData(BaseMemoryPool* pool)
		: m_pixels(pool)
		, m_s3tcPixels(pool)
		, m_astcPixels(pool)
	{
	}
};

class Mipmap
{
public:
	/// One surface for each layer ore one per face or a single volume if it's a 3D texture.
	DynamicArrayRaii<SurfaceOrVolumeData> m_surfacesOrVolume;

	Mipmap(BaseMemoryPool* pool)
		: m_surfacesOrVolume(pool)
	{
	}
};

/// Image importer context.
class ImageImporterContext
{
public:
	DynamicArrayRaii<Mipmap> m_mipmaps;
	U32 m_width = 0;
	U32 m_height = 0;
	U32 m_depth = 0;
	U32 m_faceCount = 0;
	U32 m_layerCount = 0;
	U32 m_channelCount = 0;
	U32 m_pixelSize = 0; ///< Texel size of an uncompressed image.
	Bool m_hdr = false;

	ImageImporterContext(BaseMemoryPool* pool)
		: m_mipmaps(pool)
	{
	}

	BaseMemoryPool& getMemoryPool()
	{
		return m_mipmaps.getMemoryPool();
	}
};

class DdsPixelFormat
{
public:
	U32 m_dwSize;
	U32 m_dwFlags;
	Array<char, 4> m_dwFourCC;
	U32 m_dwRGBBitCount;
	U32 m_dwRBitMask;
	U32 m_dwGBitMask;
	U32 m_dwBBitMask;
	U32 m_dwABitMask;
};

class DdsHeader
{
public:
	Array<U8, 4> m_magic;
	U32 m_dwSize;
	U32 m_dwFlags;
	U32 m_dwHeight;
	U32 m_dwWidth;
	U32 m_dwPitchOrLinearSize;
	U32 m_dwDepth;
	U32 m_dwMipMapCount;
	Array<U32, 11> m_dwReserved1;
	DdsPixelFormat m_ddspf;
	U32 m_dwCaps;
	U32 m_dwCaps2;
	U32 m_dwCaps3;
	U32 m_dwCaps4;
	U32 m_dwReserved2;
};

enum D3D10ResourceDimension
{
	D3D10_RESOURCE_DIMENSION_UNKNOWN = 0,
	D3D10_RESOURCE_DIMENSION_BUFFER = 1,
	D3D10_RESOURCE_DIMENSION_TEXTURE1D = 2,
	D3D10_RESOURCE_DIMENSION_TEXTURE2D = 3,
	D3D10_RESOURCE_DIMENSION_TEXTURE3D = 4
};

/// Extra header for some DDS formats.
class DdsHeaderDxt10
{
public:
	U32 m_dxgiFormat;
	D3D10ResourceDimension m_resourceDimension;
	U32 m_miscFlag;
	U32 m_arraySize;
	U32 m_reserved;
};

class AstcHeader
{
public:
	Array<U8, 4> m_magic;
	U8 m_blockX;
	U8 m_blockY;
	U8 m_blockZ;
	Array<U8, 3> m_dimX;
	Array<U8, 3> m_dimY;
	Array<U8, 3> m_dimZ;
};

/// Simple class to delete a file when it goes out of scope.
class CleanupFile
{
public:
	StringRaii m_fileToDelete;

	CleanupFile(BaseMemoryPool* pool, CString filename)
		: m_fileToDelete(pool, filename)
	{
	}

	~CleanupFile()
	{
		if(!m_fileToDelete.isEmpty())
		{
			const Error err = removeFile(m_fileToDelete);
			if(!err)
			{
				ANKI_IMPORTER_LOGV("Deleted %s", m_fileToDelete.cstr());
			}
		}
	}
};

} // namespace

static Error checkConfig(const ImageImporterConfig& config)
{
#define ANKI_CFG_ASSERT(x, message) \
	do \
	{ \
		if(!(x)) \
		{ \
			ANKI_IMPORTER_LOGE(message); \
			return Error::kUserData; \
		} \
	} while(false)

	// Filenames
	ANKI_CFG_ASSERT(config.m_outFilename.getLength() > 0, "Empty output filename");

	for(CString in : config.m_inputFilenames)
	{
		ANKI_CFG_ASSERT(in.getLength() > 0, "Empty input filename");
	}

	// Type
	ANKI_CFG_ASSERT(config.m_type != ImageBinaryType::kNone, "Wrong image type");
	ANKI_CFG_ASSERT(config.m_inputFilenames.getSize() == 1 || config.m_type != ImageBinaryType::k2D,
					"2D images require only one input image");
	ANKI_CFG_ASSERT(config.m_inputFilenames.getSize() != 1 || config.m_type != ImageBinaryType::k2DArray,
					"2D array images require more than one input image");
	ANKI_CFG_ASSERT(config.m_inputFilenames.getSize() != 1 || config.m_type != ImageBinaryType::k3D,
					"3D images require more than one input image");
	ANKI_CFG_ASSERT(config.m_inputFilenames.getSize() != 6 || config.m_type != ImageBinaryType::kCube,
					"Cube images require 6 input images");

	// Compressions
	ANKI_CFG_ASSERT(config.m_compressions != ImageBinaryDataCompression::kNone, "Missing output compressions");
	ANKI_CFG_ASSERT(config.m_compressions == ImageBinaryDataCompression::kRaw || config.m_type != ImageBinaryType::k3D,
					"Can't compress 3D textures");

	// ASTC
	if(!!(config.m_compressions & ImageBinaryDataCompression::kAstc))
	{
		ANKI_CFG_ASSERT(config.m_astcBlockSize == UVec2(4u) || config.m_astcBlockSize == UVec2(8u),
						"Incorrect ASTC block sizes");
	}

	// Mip size
	ANKI_CFG_ASSERT(config.m_minMipmapDimension >= 4, "Mimpap min dimension can be less than 4");

	// Color conversions
	ANKI_CFG_ASSERT(!(config.m_linearToSRgb && config.m_sRgbToLinear),
					"Can't have a conversion to sRGB and to linear at the same time");

#undef ANKI_CFG_ASSERT
	return Error::kNone;
}

static Error identifyImage(CString filename, U32& width, U32& height, U32& channelCount, Bool& hdr)
{
	I32 iwidth, iheight, ichannelCount;
	const I ok = stbi_info(filename.cstr(), &iwidth, &iheight, &ichannelCount);
	if(!ok)
	{
		ANKI_IMPORTER_LOGE("STB failed to load file: %s", filename.cstr());
		return Error::kFunctionFailed;
	}

	const I ihdr = stbi_is_hdr(filename.cstr());

	width = U32(iwidth);
	height = U32(iheight);
	channelCount = U32(ichannelCount);
	hdr = U32(ihdr);

	return Error::kNone;
}

static Error checkInputImages(const ImageImporterConfig& config, U32& width, U32& height, U32& channelCount,
							  Bool& isHdr)
{
	width = 0;
	height = 0;
	channelCount = 0;
	isHdr = false;

	for(U32 i = 0; i < config.m_inputFilenames.getSize(); ++i)
	{
		U32 nwidth, nheight, nchannelCount;
		Bool nhdr;
		ANKI_CHECK(identifyImage(config.m_inputFilenames[i], nwidth, nheight, nchannelCount, nhdr));

		if(i == 0)
		{
			width = nwidth;
			height = nheight;
			channelCount = nchannelCount;
			isHdr = nhdr;
		}
		else if(width != nwidth || height != nheight || channelCount != nchannelCount || isHdr != nhdr)
		{
			ANKI_IMPORTER_LOGE("Input image doesn't match previous input images: %s",
							   config.m_inputFilenames[i].cstr());
			return Error::kUserData;
		}
	}

	ANKI_ASSERT(width > 0 && height > 0 && channelCount > 0);
	if(!isPowerOfTwo(width) || !isPowerOfTwo(height))
	{
		ANKI_IMPORTER_LOGE("Only power of two images are accepted");
		return Error::kUserData;
	}

	return Error::kNone;
}

static Error resizeImage(CString inImageFilename, U32 outWidth, U32 outHeight, CString tempDirectory,
						 BaseMemoryPool& pool, StringRaii& tmpFilename)
{
	U32 inWidth, inHeight, channelCount;
	Bool hdr;
	ANKI_CHECK(identifyImage(inImageFilename, inWidth, inHeight, channelCount, hdr));

	// Load
	void* inPixels;
	if(!hdr)
	{
		I32 width, height;
		inPixels = stbi_load(inImageFilename.cstr(), &width, &height, nullptr, channelCount);
	}
	else
	{
		I32 width, height;
		inPixels = stbi_loadf(inImageFilename.cstr(), &width, &height, nullptr, channelCount);
	}

	if(!inPixels)
	{
		ANKI_IMPORTER_LOGE("STB load failed: %s", inImageFilename.cstr());
		return Error::kFunctionFailed;
	}

	// Resize
	I ok;
	DynamicArrayRaii<U8> outPixels(&pool);
	if(!hdr)
	{
		outPixels.resize(outWidth * outHeight * channelCount);
		ok = stbir_resize_uint8(static_cast<const U8*>(inPixels), inWidth, inHeight, 0, outPixels.getBegin(), outWidth,
								outHeight, 0, channelCount);
	}
	else
	{
		outPixels.resize(outWidth * outHeight * channelCount * sizeof(F32));
		ok = stbir_resize_float(static_cast<const F32*>(inPixels), inWidth, inHeight, 0,
								reinterpret_cast<F32*>(outPixels.getBegin()), outWidth, outHeight, 0, channelCount);
	}

	stbi_image_free(inPixels);

	if(!ok)
	{
		ANKI_IMPORTER_LOGE("stbir_resize_xxx() failed to resize the image: %s", inImageFilename.cstr());
		return Error::kFunctionFailed;
	}

	// Store
	tmpFilename.sprintf("%s/AnKiImageImporter_%u.%s", tempDirectory.cstr(), U32(std::rand()), (hdr) ? "exr" : "png");
	ANKI_IMPORTER_LOGV("Will store: %s", tmpFilename.cstr());

	if(!hdr)
	{
		ok = stbi_write_png(tmpFilename.cstr(), outWidth, outHeight, channelCount, outPixels.getBegin(), 0);
	}
	else
	{
		const I ret = SaveEXR(reinterpret_cast<const F32*>(outPixels.getBegin()), outWidth, outHeight, channelCount, 0,
							  tmpFilename.cstr(), nullptr);
		ok = ret >= 0;
	}

	if(!ok)
	{
		ANKI_IMPORTER_LOGE("Failed to create: %s", tmpFilename.cstr());
		return Error::kFunctionFailed;
	}

	return Error::kNone;
}

static Vec3 linearToSRgb(Vec3 p)
{
	Vec3 cutoff;
	cutoff.x() = (p.x() < 0.0031308f) ? 1.0f : 0.0f;
	cutoff.y() = (p.y() < 0.0031308f) ? 1.0f : 0.0f;
	cutoff.z() = (p.z() < 0.0031308f) ? 1.0f : 0.0f;

	const Vec3 higher = 1.055f * p.pow(1.0f / 2.4f) - 0.055f;
	const Vec3 lower = p * 12.92f;
	p = higher.lerp(lower, cutoff);

	return p;
}

static Vec3 sRgbToLinear(Vec3 p)
{
	Vec3 cutoff;
	cutoff.x() = (p.x() < 0.04045f) ? 1.0f : 0.0f;
	cutoff.y() = (p.y() < 0.04045f) ? 1.0f : 0.0f;
	cutoff.z() = (p.z() < 0.04045f) ? 1.0f : 0.0f;

	const Vec3 higher = ((p + 0.055f) / 1.055f).pow(2.4f);
	const Vec3 lower = p / 12.92f;
	const Vec3 out = higher.lerp(lower, cutoff);
	return out;
}

template<typename TVec, typename TFunc>
static void linearToSRgbBatch(WeakArray<TVec> pixels, TFunc func)
{
	using S = typename TVec::Scalar;

	for(TVec& pixel : pixels)
	{
		Vec3 p;
		if(std::is_same<S, U8>::value)
		{
			p.x() = F32(pixel.x()) / 255.0f;
			p.y() = F32(pixel.y()) / 255.0f;
			p.z() = F32(pixel.z()) / 255.0f;
		}
		else
		{
			ANKI_ASSERT((std::is_same<S, F32>::value));
			p.x() = F32(pixel.x());
			p.y() = F32(pixel.y());
			p.z() = F32(pixel.z());
		}

		p = func(p);

		if(std::is_same<S, U8>::value)
		{
			pixel.x() = S(p.x() * 255.0f);
			pixel.y() = S(p.y() * 255.0f);
			pixel.z() = S(p.z() * 255.0f);
		}
		else
		{
			pixel.x() = S(p.x());
			pixel.y() = S(p.y());
			pixel.z() = S(p.z());
		}
	}
}

static void applyScaleAndBias(WeakArray<Vec3> pixels, Vec3 scale, Vec3 bias)
{
	for(Vec3& pixel : pixels)
	{
		pixel = pixel * scale + bias;
	}
}

static Error loadFirstMipmap(const ImageImporterConfig& config, ImageImporterContext& ctx)
{
	BaseMemoryPool& pool = ctx.getMemoryPool();

	ctx.m_mipmaps.emplaceBack(&pool);
	Mipmap& mip0 = ctx.m_mipmaps[0];

	if(ctx.m_depth > 1)
	{
		mip0.m_surfacesOrVolume.create(1, &pool);
		mip0.m_surfacesOrVolume[0].m_pixels.create(ctx.m_pixelSize * ctx.m_width * ctx.m_height * ctx.m_depth);
	}
	else
	{
		mip0.m_surfacesOrVolume.create(ctx.m_faceCount * ctx.m_layerCount, &pool);
		ANKI_ASSERT(mip0.m_surfacesOrVolume.getSize() == config.m_inputFilenames.getSize());

		for(U32 f = 0; f < ctx.m_faceCount; ++f)
		{
			for(U32 l = 0; l < ctx.m_layerCount; ++l)
			{
				mip0.m_surfacesOrVolume[l * ctx.m_faceCount + f].m_pixels.create(ctx.m_pixelSize * ctx.m_width
																				 * ctx.m_height);
			}
		}
	}

	for(U32 i = 0; i < config.m_inputFilenames.getSize(); ++i)
	{
		I32 width, height;
		stbi_set_flip_vertically_on_load_thread(config.m_flipImage);
		void* data;
		if(!ctx.m_hdr)
		{
			data = stbi_load(config.m_inputFilenames[i].cstr(), &width, &height, nullptr, ctx.m_channelCount);
		}
		else
		{
			data = stbi_loadf(config.m_inputFilenames[i].cstr(), &width, &height, nullptr, ctx.m_channelCount);
		}

		if(!data)
		{
			ANKI_IMPORTER_LOGE("STB load failed: %s", config.m_inputFilenames[i].cstr());
			return Error::kFunctionFailed;
		}

		const PtrSize dataSize = PtrSize(ctx.m_width) * ctx.m_height * ctx.m_pixelSize;

		// To conversions in place
		if(config.m_linearToSRgb)
		{
			ANKI_IMPORTER_LOGV("Will convert linear to sRGB");

			if(ctx.m_channelCount == 3)
			{
				if(!ctx.m_hdr)
				{
					linearToSRgbBatch(WeakArray<U8Vec3>(static_cast<U8Vec3*>(data), ctx.m_width * ctx.m_height),
									  linearToSRgb);
				}
				else
				{
					linearToSRgbBatch(WeakArray<Vec3>(static_cast<Vec3*>(data), ctx.m_width * ctx.m_height),
									  linearToSRgb);
				}
			}
			else
			{
				ANKI_ASSERT(ctx.m_channelCount == 4);
				if(!ctx.m_hdr)
				{
					linearToSRgbBatch(WeakArray<U8Vec4>(static_cast<U8Vec4*>(data), ctx.m_width * ctx.m_height),
									  linearToSRgb);
				}
				else
				{
					linearToSRgbBatch(WeakArray<Vec4>(static_cast<Vec4*>(data), ctx.m_width * ctx.m_height),
									  linearToSRgb);
				}
			}
		}
		else if(config.m_sRgbToLinear)
		{
			ANKI_IMPORTER_LOGV("Will convert sRGB to linear");

			if(ctx.m_channelCount == 3)
			{
				if(!ctx.m_hdr)
				{
					linearToSRgbBatch(WeakArray<U8Vec3>(static_cast<U8Vec3*>(data), ctx.m_width * ctx.m_height),
									  sRgbToLinear);
				}
				else
				{
					linearToSRgbBatch(WeakArray<Vec3>(static_cast<Vec3*>(data), ctx.m_width * ctx.m_height),
									  sRgbToLinear);
				}
			}
			else
			{
				ANKI_ASSERT(ctx.m_channelCount == 4);
				if(!ctx.m_hdr)
				{
					linearToSRgbBatch(WeakArray<U8Vec4>(static_cast<U8Vec4*>(data), ctx.m_width * ctx.m_height),
									  sRgbToLinear);
				}
				else
				{
					linearToSRgbBatch(WeakArray<Vec4>(static_cast<Vec4*>(data), ctx.m_width * ctx.m_height),
									  sRgbToLinear);
				}
			}
		}

		if(ctx.m_hdr && (config.m_hdrScale != Vec3(1.0f) || config.m_hdrBias != Vec3(0.0f)))
		{
			ANKI_IMPORTER_LOGV("Will apply scale and/or bias to the image");
			applyScaleAndBias(WeakArray(static_cast<Vec3*>(data), ctx.m_width * ctx.m_height), config.m_hdrScale,
							  config.m_hdrBias);
		}

		if(ctx.m_depth > 1)
		{
			memcpy(mip0.m_surfacesOrVolume[0].m_pixels.getBegin() + i * dataSize, data, dataSize);
		}
		else
		{
			memcpy(mip0.m_surfacesOrVolume[i].m_pixels.getBegin(), data, dataSize);
		}

		stbi_image_free(data);
	}

	return Error::kNone;
}

template<typename TStorageVec>
static void generateSurfaceMipmap(ConstWeakArray<U8, PtrSize> inBuffer, U32 inWidth, U32 inHeight,
								  WeakArray<U8, PtrSize> outBuffer)
{
	constexpr U32 channelCount = TStorageVec::getSize();
	using FVecType = typename std::conditional_t<channelCount == 3, Vec3, Vec4>;

	const ConstWeakArray<TStorageVec, PtrSize> inPixels(reinterpret_cast<const TStorageVec*>(&inBuffer[0]),
														inBuffer.getSizeInBytes() / sizeof(TStorageVec));
	WeakArray<TStorageVec, PtrSize> outPixels(reinterpret_cast<TStorageVec*>(&outBuffer[0]),
											  outBuffer.getSizeInBytes() / sizeof(TStorageVec));

	const U32 outWidth = inWidth >> 1;
	const U32 outHeight = inHeight >> 1;

	for(U32 h = 0; h < outHeight; ++h)
	{
		for(U32 w = 0; w < outWidth; ++w)
		{
			// Gather input
			FVecType average(0.0f);
			for(U32 y = 0; y < 2; ++y)
			{
				for(U32 x = 0; x < 2; ++x)
				{
					const U32 idx = (h * 2 + y) * inWidth + (w * 2 + x);
					const TStorageVec inPixel = inPixels[idx];
					for(U32 c = 0; c < channelCount; ++c)
					{
						average[c] += F32(inPixel[c]) * 0.25f;
					}
				}
			}

			TStorageVec uaverage;
			for(U32 c = 0; c < channelCount; ++c)
			{
				uaverage[c] = typename TStorageVec::Scalar(average[c]);
			}

			// Store
			const U32 idx = h * outWidth + w;
			outPixels[idx] = uaverage;
		}
	}
}

static Error compressS3tc(BaseMemoryPool& pool, CString tempDirectory, CString compressonatorFilename,
						  ConstWeakArray<U8, PtrSize> inPixels, U32 inWidth, U32 inHeight, U32 channelCount, Bool hdr,
						  WeakArray<U8, PtrSize> outPixels)
{
	ANKI_ASSERT(inPixels.getSizeInBytes()
				== PtrSize(inWidth) * inHeight * channelCount * ((hdr) ? sizeof(F32) : sizeof(U8)));
	ANKI_ASSERT(inWidth > 0 && isPowerOfTwo(inWidth) && inHeight > 0 && isPowerOfTwo(inHeight));
	ANKI_ASSERT(outPixels.getSizeInBytes()
				== PtrSize((hdr || channelCount == 4) ? 16 : 8) * (inWidth / 4) * (inHeight / 4));

	// Create a PNG image to feed to the compressor
	StringRaii tmpFilename(&pool);
	tmpFilename.sprintf("%s/AnKiImageImporter_%u.%s", tempDirectory.cstr(), U32(std::rand()), (hdr) ? "exr" : "png");
	ANKI_IMPORTER_LOGV("Will store: %s", tmpFilename.cstr());
	Bool saveTmpImageOk = false;
	if(!hdr)
	{
		const I ok = stbi_write_png(tmpFilename.cstr(), inWidth, inHeight, channelCount, inPixels.getBegin(), 0);
		saveTmpImageOk = !!ok;
	}
	else
	{
		const I ret = SaveEXR(reinterpret_cast<const F32*>(inPixels.getBegin()), inWidth, inHeight, channelCount, 0,
							  tmpFilename.cstr(), nullptr);
		saveTmpImageOk = ret >= 0;
	}

	if(!saveTmpImageOk)
	{
		ANKI_IMPORTER_LOGE("Failed to create: %s", tmpFilename.cstr());
		return Error::kFunctionFailed;
	}
	CleanupFile tmpCleanup(&pool, tmpFilename);

	// Invoke the compressor process
	StringRaii ddsFilename(&pool);
	ddsFilename.sprintf("%s/AnKiImageImporter_%u.dds", tempDirectory.cstr(), U32(std::rand()));
	Process proc;
	Array<CString, 5> args;
	U32 argCount = 0;
	args[argCount++] = "-nomipmap";
	args[argCount++] = "-fd";
	args[argCount++] = (hdr) ? "BC6H" : ((channelCount == 3) ? "BC1" : "BC3");
	args[argCount++] = tmpFilename;
	args[argCount++] = ddsFilename;

	ANKI_IMPORTER_LOGV("Will invoke process: compressonatorcli %s %s %s %s %s", args[0].cstr(), args[1].cstr(),
					   args[2].cstr(), args[3].cstr(), args[4].cstr());
	ANKI_CHECK(proc.start(compressonatorFilename, args));
	CleanupFile ddsCleanup(&pool, ddsFilename);
	ProcessStatus status;
	I32 exitCode;
	ANKI_CHECK(proc.wait(60.0_sec, &status, &exitCode));

	if(!(status == ProcessStatus::kNotRunning && exitCode == 0))
	{
		StringRaii errStr(&pool);
		if(exitCode != 0)
		{
			ANKI_CHECK(proc.readFromStdout(errStr));
		}

		if(errStr.isEmpty())
		{
			errStr = "Unknown error";
		}

		ANKI_IMPORTER_LOGE("Invoking compressor process failed: %s", errStr.cstr());
		return Error::kFunctionFailed;
	}

	// Read the DDS file
	File ddsFile;
	ANKI_CHECK(ddsFile.open(ddsFilename, FileOpenFlag::kRead | FileOpenFlag::kBinary));
	DdsHeader ddsHeader;
	ANKI_CHECK(ddsFile.read(&ddsHeader, sizeof(DdsHeader)));

	if(!hdr && channelCount == 3 && memcmp(&ddsHeader.m_ddspf.m_dwFourCC[0], "DXT1", 4) != 0)
	{
		ANKI_IMPORTER_LOGE("Incorrect format. Expecting DXT1");
		return Error::kFunctionFailed;
	}

	if(!hdr && channelCount == 4 && memcmp(&ddsHeader.m_ddspf.m_dwFourCC[0], "DXT5", 4) != 0)
	{
		ANKI_IMPORTER_LOGE("Incorrect format. Expecting DXT5");
		return Error::kFunctionFailed;
	}

	if(hdr && memcmp(&ddsHeader.m_ddspf.m_dwFourCC[0], "DX10", 4) != 0)
	{
		ANKI_IMPORTER_LOGE("Incorrect format. Expecting BC6H");
		return Error::kFunctionFailed;
	}

	if(ddsHeader.m_dwWidth != inWidth || ddsHeader.m_dwHeight != inHeight)
	{
		ANKI_IMPORTER_LOGE("Incorrect DDS image size");
		return Error::kFunctionFailed;
	}

	if(hdr)
	{
		DdsHeaderDxt10 dxt10Header;
		ANKI_CHECK(ddsFile.read(&dxt10Header, sizeof(dxt10Header)));
	}

	ANKI_CHECK(ddsFile.read(outPixels.getBegin(), outPixels.getSizeInBytes()));

	return Error::kNone;
}

static Error compressAstc(BaseMemoryPool& pool, CString tempDirectory, CString astcencFilename,
						  ConstWeakArray<U8, PtrSize> inPixels, U32 inWidth, U32 inHeight, U32 inChannelCount,
						  UVec2 blockSize, Bool hdr, WeakArray<U8, PtrSize> outPixels)
{
	[[maybe_unused]] const PtrSize blockBytes = 16;
	ANKI_ASSERT(inPixels.getSizeInBytes()
				== PtrSize(inWidth) * inHeight * inChannelCount * ((hdr) ? sizeof(F32) : sizeof(U8)));
	ANKI_ASSERT(inWidth > 0 && isPowerOfTwo(inWidth) && inHeight > 0 && isPowerOfTwo(inHeight));
	ANKI_ASSERT(outPixels.getSizeInBytes() == blockBytes * (inWidth / blockSize.x()) * (inHeight / blockSize.y()));

	// Create a BMP image to feed to the astcebc
	StringRaii tmpFilename(&pool);
	tmpFilename.sprintf("%s/AnKiImageImporter_%u.%s", tempDirectory.cstr(), U32(std::rand()), (hdr) ? "exr" : "png");
	ANKI_IMPORTER_LOGV("Will store: %s", tmpFilename.cstr());
	Bool saveTmpImageOk = false;
	if(!hdr)
	{
		const I ok = stbi_write_png(tmpFilename.cstr(), inWidth, inHeight, inChannelCount, inPixels.getBegin(), 0);
		saveTmpImageOk = !!ok;
	}
	else
	{
		const I ret = SaveEXR(reinterpret_cast<const F32*>(inPixels.getBegin()), inWidth, inHeight, inChannelCount, 0,
							  tmpFilename.cstr(), nullptr);
		saveTmpImageOk = ret >= 0;
	}

	if(!saveTmpImageOk)
	{
		ANKI_IMPORTER_LOGE("Failed to create: %s", tmpFilename.cstr());
		return Error::kFunctionFailed;
	}
	CleanupFile pngCleanup(&pool, tmpFilename);

	// Invoke the compressor process
	StringRaii astcFilename(&pool);
	astcFilename.sprintf("%s/AnKiImageImporter_%u.astc", tempDirectory.cstr(), U32(std::rand()));
	StringRaii blockStr(&pool);
	blockStr.sprintf("%ux%u", blockSize.x(), blockSize.y());
	Process proc;
	Array<CString, 5> args;
	U32 argCount = 0;
	args[argCount++] = (!hdr) ? "-cl" : "-ch";
	args[argCount++] = tmpFilename;
	args[argCount++] = astcFilename;
	args[argCount++] = blockStr;
	args[argCount++] = "-fast";

	ANKI_IMPORTER_LOGV("Will invoke process: astcenc-avx2 %s %s %s %s %s", args[0].cstr(), args[1].cstr(),
					   args[2].cstr(), args[3].cstr(), args[4].cstr());
	ANKI_CHECK(proc.start(astcencFilename, args));

	CleanupFile astcCleanup(&pool, astcFilename);
	ProcessStatus status;
	I32 exitCode;
	ANKI_CHECK(proc.wait(60.0_sec, &status, &exitCode));

	if(!(status == ProcessStatus::kNotRunning && exitCode == 0))
	{
		StringRaii errStr(&pool);
		if(exitCode != 0)
		{
			ANKI_CHECK(proc.readFromStdout(errStr));
		}

		if(errStr.isEmpty())
		{
			errStr = "Unknown error";
		}

		ANKI_IMPORTER_LOGE("Invoking astcenc-avx2 process failed: %s", errStr.cstr());
		return Error::kFunctionFailed;
	}

	// Read the astc file
	File astcFile;
	ANKI_CHECK(astcFile.open(astcFilename, FileOpenFlag::kRead | FileOpenFlag::kBinary));
	AstcHeader header;
	ANKI_CHECK(astcFile.read(&header, sizeof(header)));

	auto unpackBytes = [](U8 a, U8 b, U8 c, U8 d) -> U32 {
		return (U32(a)) + (U32(b) << 8) + (U32(c) << 16) + (U32(d) << 24);
	};

	const U32 magicval = unpackBytes(header.m_magic[0], header.m_magic[1], header.m_magic[2], header.m_magic[3]);
	if(magicval != 0x5CA1AB13)
	{
		ANKI_IMPORTER_LOGE("astcenc-avx2 produced a file with wrong magic");
		return Error::kFunctionFailed;
	}

	const U32 blockx = max<U32>(header.m_blockX, 1u);
	const U32 blocky = max<U32>(header.m_blockY, 1u);
	const U32 blockz = max<U32>(header.m_blockZ, 1u);
	if(blockx != blockSize.x() || blocky != blockSize.y() || blockz != 1)
	{
		ANKI_IMPORTER_LOGE("astcenc-avx2 with wrong block size");
		return Error::kFunctionFailed;
	}

	const U32 dimx = unpackBytes(header.m_dimX[0], header.m_dimX[1], header.m_dimX[2], 0);
	const U32 dimy = unpackBytes(header.m_dimY[0], header.m_dimY[1], header.m_dimY[2], 0);
	const U32 dimz = unpackBytes(header.m_dimZ[0], header.m_dimZ[1], header.m_dimZ[2], 0);
	if(dimx != inWidth || dimy != inHeight || dimz != 1)
	{
		ANKI_IMPORTER_LOGE("astcenc-avx2 with wrong image size");
		return Error::kFunctionFailed;
	}

	ANKI_CHECK(astcFile.read(outPixels.getBegin(), outPixels.getSizeInBytes()));

	return Error::kNone;
}

static Error storeAnkiImage(const ImageImporterConfig& config, const ImageImporterContext& ctx)
{
	ANKI_IMPORTER_LOGV("Storing to %s", config.m_outFilename.cstr());

	File outFile;
	ANKI_CHECK(outFile.open(config.m_outFilename, FileOpenFlag::kBinary | FileOpenFlag::kWrite));

	// Header
	ImageBinaryHeader header = {};
	memcpy(&header.m_magic[0], &kImageMagic[0], sizeof(header.m_magic));
	header.m_width = ctx.m_width;
	header.m_height = ctx.m_height;
	header.m_depthOrLayerCount = max(ctx.m_layerCount, ctx.m_depth);
	header.m_type = config.m_type;
	if(ctx.m_hdr)
	{
		header.m_colorFormat =
			(ctx.m_channelCount == 3) ? ImageBinaryColorFormat::kRgbFloat : ImageBinaryColorFormat::kRgbaFloat;
	}
	else
	{
		header.m_colorFormat =
			(ctx.m_channelCount == 3) ? ImageBinaryColorFormat::kRgb8 : ImageBinaryColorFormat::kRgba8;
	}
	header.m_compressionMask = config.m_compressions;
	header.m_isNormal = false;
	header.m_mipmapCount = ctx.m_mipmaps.getSize();
	header.m_astcBlockSizeX = config.m_astcBlockSize.x();
	header.m_astcBlockSizeY = config.m_astcBlockSize.y();
	ANKI_CHECK(outFile.write(&header, sizeof(header)));

	// Write RAW
	if(!!(config.m_compressions & ImageBinaryDataCompression::kRaw))
	{
		ANKI_IMPORTER_LOGV("Storing RAW");

		// for(I32 mip = I32(ctx.m_mipmaps.getSize()) - 1; mip >= 0; --mip)
		for(U32 mip = 0; mip < ctx.m_mipmaps.getSize(); ++mip)
		{
			for(U32 l = 0; l < ctx.m_layerCount; ++l)
			{
				for(U32 f = 0; f < ctx.m_faceCount; ++f)
				{
					const U32 idx = l * ctx.m_faceCount + f;
					const ConstWeakArray<U8, PtrSize> pixels = ctx.m_mipmaps[mip].m_surfacesOrVolume[idx].m_pixels;
					ANKI_CHECK(outFile.write(&pixels[0], pixels.getSizeInBytes()));
				}
			}
		}
	}

	// Write S3TC
	if(!!(config.m_compressions & ImageBinaryDataCompression::kS3tc))
	{
		ANKI_IMPORTER_LOGV("Storing S3TC");

		// for(I32 mip = I32(ctx.m_mipmaps.getSize()) - 1; mip >= 0; --mip)
		for(U32 mip = 0; mip < ctx.m_mipmaps.getSize(); ++mip)
		{
			for(U32 l = 0; l < ctx.m_layerCount; ++l)
			{
				for(U32 f = 0; f < ctx.m_faceCount; ++f)
				{
					const U32 idx = l * ctx.m_faceCount + f;
					const ConstWeakArray<U8, PtrSize> pixels = ctx.m_mipmaps[mip].m_surfacesOrVolume[idx].m_s3tcPixels;
					ANKI_CHECK(outFile.write(&pixels[0], pixels.getSizeInBytes()));
				}
			}
		}
	}

	// Write ASTC
	if(!!(config.m_compressions & ImageBinaryDataCompression::kAstc))
	{
		ANKI_IMPORTER_LOGV("Storing ASTC");

		// for(I32 mip = I32(ctx.m_mipmaps.getSize()) - 1; mip >= 0; --mip)
		for(U32 mip = 0; mip < ctx.m_mipmaps.getSize(); ++mip)
		{
			for(U32 l = 0; l < ctx.m_layerCount; ++l)
			{
				for(U32 f = 0; f < ctx.m_faceCount; ++f)
				{
					const U32 idx = l * ctx.m_faceCount + f;
					const ConstWeakArray<U8, PtrSize> pixels = ctx.m_mipmaps[mip].m_surfacesOrVolume[idx].m_astcPixels;
					ANKI_CHECK(outFile.write(&pixels[0], pixels.getSizeInBytes()));
				}
			}
		}
	}

	return Error::kNone;
}

static Error importImageInternal(const ImageImporterConfig& configOriginal)
{
	BaseMemoryPool& pool = *configOriginal.m_pool;
	ImageImporterConfig config = configOriginal;

	config.m_minMipmapDimension = max(config.m_minMipmapDimension, 4u);
	if(!!(config.m_compressions & ImageBinaryDataCompression::kAstc))
	{
		config.m_minMipmapDimension = max(config.m_minMipmapDimension, config.m_astcBlockSize.x());
		config.m_minMipmapDimension = max(config.m_minMipmapDimension, config.m_astcBlockSize.y());
	}

	// Checks
	ANKI_CHECK(checkConfig(config));
	U32 width, height, channelCount;
	Bool isHdr;
	ANKI_CHECK(checkInputImages(config, width, height, channelCount, isHdr));

	// Resize
	DynamicArrayRaii<StringRaii> newFilenames(&pool);
	DynamicArrayRaii<CString> newFilenamesCString(&pool);
	DynamicArrayRaii<CleanupFile> resizedImagesCleanup(&pool);
	if(width < config.m_minMipmapDimension || height < config.m_minMipmapDimension)
	{
		const U32 newWidth = max(width, config.m_minMipmapDimension);
		const U32 newHeight = max(height, config.m_minMipmapDimension);

		ANKI_IMPORTER_LOGV("Image is smaller than the min mipmap dimension. Will resize it to %ux%u", newWidth,
						   newHeight);

		newFilenames.resize(config.m_inputFilenames.getSize(), StringRaii(&pool));
		newFilenamesCString.resize(config.m_inputFilenames.getSize());

		for(U32 i = 0; i < config.m_inputFilenames.getSize(); ++i)
		{
			ANKI_CHECK(resizeImage(config.m_inputFilenames[i], newWidth, newHeight, config.m_tempDirectory, pool,
								   newFilenames[i]));

			newFilenamesCString[i] = newFilenames[i];
			resizedImagesCleanup.emplaceBack(&pool, newFilenames[i]);
		}

		// Override config
		config.m_inputFilenames = newFilenamesCString;
		width = newWidth;
		height = newHeight;
	}

	// Init image
	ImageImporterContext ctx(&pool);

	ctx.m_width = width;
	ctx.m_height = height;
	ctx.m_depth = (config.m_type == ImageBinaryType::k3D) ? config.m_inputFilenames.getSize() : 1;
	ctx.m_faceCount = (config.m_type == ImageBinaryType::kCube) ? 6 : 1;
	ctx.m_layerCount = (config.m_type == ImageBinaryType::k2DArray) ? config.m_inputFilenames.getSize() : 1;
	ctx.m_hdr = isHdr;

	U32 desiredChannelCount;
	if(isHdr && !!(config.m_compressions & ImageBinaryDataCompression::kS3tc))
	{
		// BC6H doesn't have a 4th channel
		if(channelCount != 3)
		{
			ANKI_IMPORTER_LOGW("Input images have alpha but that can't be supported with BC6H");
		}

		if(!!(config.m_compressions & ImageBinaryDataCompression::kRaw))
		{
			ANKI_IMPORTER_LOGE("Can't support both BC6H (which is 3 component) and RAW which requires 4 compoments");
			return Error::kUserData;
		}

		desiredChannelCount = 3;
	}
	else if(!!(config.m_compressions & ImageBinaryDataCompression::kRaw))
	{
		// Always ask for 4 components because desktop GPUs don't always like 3
		desiredChannelCount = 4;
	}
	else if(config.m_noAlpha || channelCount == 1)
	{
		// no alpha or 1 component grey
		desiredChannelCount = 3;
	}
	else if(channelCount == 2)
	{
		// grey with alpha
		desiredChannelCount = 4;
	}
	else
	{
		desiredChannelCount = channelCount;
	}

	ctx.m_channelCount = desiredChannelCount;
	ctx.m_pixelSize = ctx.m_channelCount * U32((isHdr) ? sizeof(F32) : sizeof(U8));

	// Load first mip from the files
	ANKI_CHECK(loadFirstMipmap(config, ctx));

	// Generate mipmaps
	const U32 mipCount =
		min(config.m_mipmapCount, (config.m_type == ImageBinaryType::k3D)
									  ? computeMaxMipmapCount3d(width, height, ctx.m_depth, config.m_minMipmapDimension)
									  : computeMaxMipmapCount2d(width, height, config.m_minMipmapDimension));
	for(U32 mip = 1; mip < mipCount; ++mip)
	{
		ctx.m_mipmaps.emplaceBack(&pool);

		if(config.m_type != ImageBinaryType::k3D)
		{
			ctx.m_mipmaps[mip].m_surfacesOrVolume.create(ctx.m_faceCount * ctx.m_layerCount, &pool);
			for(U32 l = 0; l < ctx.m_layerCount; ++l)
			{
				for(U32 f = 0; f < ctx.m_faceCount; ++f)
				{
					const U32 idx = l * ctx.m_faceCount + f;
					const SurfaceOrVolumeData& inSurface = ctx.m_mipmaps[mip - 1].m_surfacesOrVolume[idx];
					SurfaceOrVolumeData& outSurface = ctx.m_mipmaps[mip].m_surfacesOrVolume[idx];
					outSurface.m_pixels.create((ctx.m_width >> mip) * (ctx.m_height >> mip) * ctx.m_pixelSize);

					if(ctx.m_channelCount == 3)
					{
						if(ctx.m_hdr)
						{
							generateSurfaceMipmap<Vec3>(ConstWeakArray<U8, PtrSize>(inSurface.m_pixels),
														ctx.m_width >> (mip - 1), ctx.m_height >> (mip - 1),
														WeakArray<U8, PtrSize>(outSurface.m_pixels));
						}
						else
						{
							generateSurfaceMipmap<U8Vec3>(ConstWeakArray<U8, PtrSize>(inSurface.m_pixels),
														  ctx.m_width >> (mip - 1), ctx.m_height >> (mip - 1),
														  WeakArray<U8, PtrSize>(outSurface.m_pixels));
						}
					}
					else
					{
						ANKI_ASSERT(ctx.m_channelCount == 4);

						if(ctx.m_hdr)
						{
							generateSurfaceMipmap<Vec4>(ConstWeakArray<U8, PtrSize>(inSurface.m_pixels),
														ctx.m_width >> (mip - 1), ctx.m_height >> (mip - 1),
														WeakArray<U8, PtrSize>(outSurface.m_pixels));
						}
						else
						{
							generateSurfaceMipmap<U8Vec4>(ConstWeakArray<U8, PtrSize>(inSurface.m_pixels),
														  ctx.m_width >> (mip - 1), ctx.m_height >> (mip - 1),
														  WeakArray<U8, PtrSize>(outSurface.m_pixels));
						}
					}
				}
			}
		}
		else
		{
			ANKI_ASSERT(!"TODO");
		}
	}

	// Compress
	if(!!(config.m_compressions & ImageBinaryDataCompression::kS3tc))
	{
		ANKI_IMPORTER_LOGV("Will compress in S3TC");

		for(U32 mip = 0; mip < mipCount; ++mip)
		{
			for(U32 l = 0; l < ctx.m_layerCount; ++l)
			{
				for(U32 f = 0; f < ctx.m_faceCount; ++f)
				{
					const U32 idx = l * ctx.m_faceCount + f;
					SurfaceOrVolumeData& surface = ctx.m_mipmaps[mip].m_surfacesOrVolume[idx];

					const U32 width = ctx.m_width >> mip;
					const U32 height = ctx.m_height >> mip;
					const PtrSize blockSize = (ctx.m_hdr || ctx.m_channelCount == 4) ? 16 : 8;
					const PtrSize s3tcImageSize = blockSize * (width / 4) * (height / 4);

					surface.m_s3tcPixels.create(s3tcImageSize);

					ANKI_CHECK(compressS3tc(pool, config.m_tempDirectory, config.m_compressonatorFilename,
											ConstWeakArray<U8, PtrSize>(surface.m_pixels), width, height,
											ctx.m_channelCount, ctx.m_hdr,
											WeakArray<U8, PtrSize>(surface.m_s3tcPixels)));
				}
			}
		}
	}

	if(!!(config.m_compressions & ImageBinaryDataCompression::kAstc))
	{
		ANKI_IMPORTER_LOGV("Will compress in ASTC");

		for(U32 mip = 0; mip < mipCount; ++mip)
		{
			for(U32 l = 0; l < ctx.m_layerCount; ++l)
			{
				for(U32 f = 0; f < ctx.m_faceCount; ++f)
				{
					const U32 idx = l * ctx.m_faceCount + f;
					SurfaceOrVolumeData& surface = ctx.m_mipmaps[mip].m_surfacesOrVolume[idx];

					const U32 width = ctx.m_width >> mip;
					const U32 height = ctx.m_height >> mip;
					const PtrSize blockSize = 16;
					const PtrSize astcImageSize =
						blockSize * (width / config.m_astcBlockSize.x()) * (height / config.m_astcBlockSize.y());

					surface.m_astcPixels.create(astcImageSize);

					ANKI_CHECK(compressAstc(pool, config.m_tempDirectory, config.m_astcencFilename,
											ConstWeakArray<U8, PtrSize>(surface.m_pixels), width, height,
											ctx.m_channelCount, config.m_astcBlockSize, ctx.m_hdr,
											WeakArray<U8, PtrSize>(surface.m_astcPixels)));
				}
			}
		}
	}

	if(!!(config.m_compressions & ImageBinaryDataCompression::kEtc))
	{
		ANKI_ASSERT(!"TODO");
	}

	// Store the image
	ANKI_CHECK(storeAnkiImage(config, ctx));

	return Error::kNone;
}

Error importImage(const ImageImporterConfig& config)
{
	const Error err = importImageInternal(config);
	if(err)
	{
		ANKI_IMPORTER_LOGE("Image importing failed");
		return err;
	}

	return Error::kNone;
}

} // end namespace anki
