#include <png.h>
#include <boost/filesystem.hpp> // For file extensions
#include <boost/algorithm/string.hpp> // For to_lower
#include <fstream>
#include "Image.h"
#include "Exception.h"
#include "Logger.h"


uchar Image::tgaHeaderUncompressed[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uchar Image::tgaHeaderCompressed[12]   = {0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0};


//======================================================================================================================
// loadUncompressedTga                                                                                                 =
//======================================================================================================================
void Image::loadUncompressedTga(std::fstream& fs, uint& bpp)
{
	// read the info from header
	uchar header6[6];
	fs.read((char*)&header6[0], sizeof(header6));
	if(fs.gcount() != sizeof(header6))
	{
		throw EXCEPTION("Cannot read info header");
	}

	width  = header6[1] * 256 + header6[0];
	height = header6[3] * 256 + header6[2];
	bpp = header6[4];

	if((width <= 0) || (height <= 0) || ((bpp != 24) && (bpp != 32)))
	{
		throw EXCEPTION("Invalid image information");
	}

	// read the data
	int bytesPerPxl	= (bpp / 8);
	int imageSize = bytesPerPxl * width * height;
	data.resize(imageSize);

	fs.read(reinterpret_cast<char*>(&data[0]), imageSize);
	if(fs.gcount() != imageSize)
	{
		throw EXCEPTION("Cannot read image data");
	}

	// swap red with blue
	for(int i = 0; i < int(imageSize); i += bytesPerPxl)
	{
		uint temp = data[i];
		data[i] = data[i + 2];
		data[i + 2] = temp;
	}
}


//======================================================================================================================
// loadCompressedTga                                                                                                   =
//======================================================================================================================
void Image::loadCompressedTga(std::fstream& fs, uint& bpp)
{
	unsigned char header6[6];
	fs.read((char*)&header6[0], sizeof(header6));
	if(fs.gcount() != sizeof(header6))
	{
		throw EXCEPTION("Cannot read info header");
	}

	width  = header6[1] * 256 + header6[0];
	height = header6[3] * 256 + header6[2];
	bpp	= header6[4];

	if((width <= 0) || (height <= 0) || ((bpp != 24) && (bpp !=32)))
	{
		throw EXCEPTION("Invalid texture information");
	}


	int bytesPerPxl = (bpp / 8);
	int image_size = bytesPerPxl * width * height;
	data.resize(image_size);

	uint pixelcount = height * width;
	uint currentpixel = 0;
	uint currentbyte	= 0;
	unsigned char colorbuffer [4];

	do
	{
		unsigned char chunkheader = 0;

		fs.read((char*)&chunkheader, sizeof(unsigned char));
		if(fs.gcount() != sizeof(unsigned char))
		{
			throw EXCEPTION("Cannot read RLE header");
		}

		if(chunkheader < 128)
		{
			chunkheader++;
			for(int counter = 0; counter < chunkheader; counter++)
			{
				fs.read((char*)&colorbuffer[0], bytesPerPxl);
				if(fs.gcount() != bytesPerPxl)
				{
					throw EXCEPTION("Cannot read image data");
				}

				data[currentbyte		] = colorbuffer[2];
				data[currentbyte + 1] = colorbuffer[1];
				data[currentbyte + 2] = colorbuffer[0];

				if(bytesPerPxl == 4)
				{
					data[currentbyte + 3] = colorbuffer[3];
				}

				currentbyte += bytesPerPxl;
				currentpixel++;

				if(currentpixel > pixelcount)
				{
					throw EXCEPTION("Too many pixels read");
				}
			}
		}
		else
		{
			chunkheader -= 127;
			fs.read((char*)&colorbuffer[0], bytesPerPxl);
			if(fs.gcount() != bytesPerPxl)
			{
				throw EXCEPTION("Cannot read from file");
			}

			for(int counter = 0; counter < chunkheader; counter++)
			{
				data[currentbyte] = colorbuffer[2];
				data[currentbyte+1] = colorbuffer[1];
				data[currentbyte+2] = colorbuffer[0];

				if(bytesPerPxl == 4)
				{
					data[currentbyte + 3] = colorbuffer[3];
				}

				currentbyte += bytesPerPxl;
				currentpixel++;

				if(currentpixel > pixelcount)
				{
					throw EXCEPTION("Too many pixels read");
				}
			}
		}
	} while(currentpixel < pixelcount);
}


//======================================================================================================================
// loadTga                                                                                                             =
//======================================================================================================================
void Image::loadTga(const char* filename)
{
	std::fstream fs;
	char myTgaHeader[12];
	fs.open(filename, std::ios::in | std::ios::binary);
	uint bpp;

	if(!fs.is_open())
	{
		throw EXCEPTION("Cannot open file");
	}

	fs.read(&myTgaHeader[0], sizeof(myTgaHeader));
	if(fs.gcount() != sizeof(myTgaHeader))
	{
		fs.close();
		throw EXCEPTION("Cannot read file header");
	}

	if(memcmp(tgaHeaderUncompressed, &myTgaHeader[0], sizeof(myTgaHeader)) == 0)
	{
		loadUncompressedTga(fs, bpp);
	}
	else if(memcmp(tgaHeaderCompressed, &myTgaHeader[0], sizeof(myTgaHeader)) == 0)
	{
		loadCompressedTga(fs, bpp);
	}
	else
	{
		throw EXCEPTION("Invalid image header");
	}

	if(bpp == 32)
	{
		type = CT_RGBA;
	}
	else if(bpp == 24)
	{
		type = CT_RGB;
	}
	else
	{
		throw EXCEPTION("Invalid bps");
	}

	fs.close();
	dataCompression = DC_NONE;
}


//======================================================================================================================
// loadPng                                                                                                             =
//======================================================================================================================
bool Image::loadPng(const char* filename, std::string& err) throw()
{
	//
	// All locals
	//
	const uint PNG_SIG_SIZE = 8; // PNG header size
	FILE* file = NULL;
	png_structp pngPtr = NULL;
	png_infop infoPtr = NULL;
	bool ok = false;
	size_t charsRead;
	uint bitDepth;
	uint channels;
	uint rowbytes;
	uint colorType;
	Vec<png_bytep> rowPointers;

	//
	// Open file
	//
	file = fopen(filename, "rb");
	if(file == NULL)
	{
		err = "Cannot open file";
		goto cleanup;
	}

	//
	// Validate PNG header
	//
	png_byte pngsig[PNG_SIG_SIZE];
	charsRead = fread(pngsig, 1, PNG_SIG_SIZE, file);
	if(charsRead != PNG_SIG_SIZE)
	{
		err = "Cannot read PNG header";
		goto cleanup;
	}

	if(png_sig_cmp(pngsig, 0, PNG_SIG_SIZE) != 0)
	{
		err = "File not PNG image";
		goto cleanup;
	}

	//
	// Crete some PNG structs
	//
	pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!pngPtr)
	{
		throw EXCEPTION("png_create_read_struct failed");
		goto cleanup;
	}

	infoPtr = png_create_info_struct(pngPtr);
	if(!infoPtr)
	{
		err = "png_create_info_struct failed";
		goto cleanup;
	}

	//
	// Set error handling
	//
	if(setjmp(png_jmpbuf(pngPtr)))
	{
		err = "Reading PNG file failed";
		goto cleanup;
	}

	//
	// Init io
	//
	png_init_io(pngPtr, file);
	png_set_sig_bytes(pngPtr, PNG_SIG_SIZE); // PNG lib knows that we already have read the header

	//
	// Read info and make conversions
	// This loop reads info, if not acceptable it calls libpng funcs to change them and re-runs the loop
	//
	png_read_info(pngPtr, infoPtr);
	while(true)
	{
		width = png_get_image_width(pngPtr, infoPtr);
		height = png_get_image_height(pngPtr, infoPtr);
		bitDepth = png_get_bit_depth(pngPtr, infoPtr);
		channels = png_get_channels(pngPtr, infoPtr);
		colorType = png_get_color_type(pngPtr, infoPtr);

		// 1) Convert the color types
		switch(colorType)
		{
			case PNG_COLOR_TYPE_PALETTE:
				err = "Converting PNG_COLOR_TYPE_PALETTE to PNG_COLOR_TYPE_RGB or PNG_COLOR_TYPE_RGBA";
				png_set_palette_to_rgb(pngPtr);
				goto again;
				break;
			case PNG_COLOR_TYPE_GRAY:
				// do nothing
				break;
			case PNG_COLOR_TYPE_GRAY_ALPHA:
				err = "Cannot accept PNG_COLOR_TYPE_GRAY_ALPHA. Converting to PNG_COLOR_TYPE_GRAY";
				png_set_strip_alpha(pngPtr);
				goto again;
				break;
			case PNG_COLOR_TYPE_RGB:
				// do nothing
				break;
			case PNG_COLOR_TYPE_RGBA:
				// do nothing
				break;
			default:
				throw EXCEPTION("Forgot to handle a color type");
				break;
		}

		// 2) Convert the bit depths
		if(colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8)
		{
			err = "Converting bit depth";
			png_set_gray_1_2_4_to_8(pngPtr);
			goto again;
		}

		if(bitDepth > 8)
		{
			err = "Converting bit depth";
			png_set_strip_16(pngPtr);
		}

		break;

		again:
			png_read_update_info(pngPtr, infoPtr);
	}

	// Sanity checks
	if((bitDepth != 8) ||
		 (colorType != PNG_COLOR_TYPE_GRAY && colorType != PNG_COLOR_TYPE_RGB && colorType != PNG_COLOR_TYPE_RGBA))
	{
		err = "Sanity checks failed";
		goto cleanup;
	}

	//
	// Read this sucker
	//
	rowbytes = png_get_rowbytes(pngPtr, infoPtr);

	rowPointers.resize(height * sizeof(png_bytep));

	data.resize(rowbytes * height);

	for (uint i = 0; i < height; ++i)
		rowPointers[height - 1 - i] = &data[i * rowbytes];

	png_read_image(pngPtr, &rowPointers[0]);

	//
	// Finalize
	//
	switch(colorType)
	{
		case PNG_COLOR_TYPE_GRAY:
			type = CT_R;
			break;
		case PNG_COLOR_TYPE_RGB:
			type = CT_RGB;
			break;
		case PNG_COLOR_TYPE_RGBA:
			type = CT_RGBA;
			break;
		default:
			err = "See file";
			goto cleanup;
	}

	ok = true;

	//
	// Cleanup
	//
	cleanup:

	if(pngPtr)
	{
		if(infoPtr)
		{
			png_destroy_read_struct(&pngPtr, &infoPtr, NULL);
		}
		else
		{
			png_destroy_read_struct(&pngPtr, NULL, NULL);
		}
	}

	if(file)
	{
		fclose(file);
	}

	dataCompression = DC_NONE;
	return ok;
}


//======================================================================================================================
// load                                                                                                                =
//======================================================================================================================
void Image::load(const char* filename)
{
	// get the extension
	std::string ext = boost::filesystem::path(filename).extension();
	boost::to_lower(ext);


	// load from this extension
	try
	{
		if(ext == ".tga")
		{
			loadTga(filename);
		}
		else if(ext == ".png")
		{
			std::string err;
			if(!loadPng(filename, err))
			{
				throw EXCEPTION(err);
			}
		}
		else if(ext == ".dds")
		{
			loadDds(filename);
		}
		else
		{
			throw EXCEPTION("Unsupported extension");
		}
	}
	catch(std::exception& e)
	{
		throw EXCEPTION("File \"" + filename + "\": " + e.what());
	}
}


//======================================================================================================================
// DDS                                                                                                                 =
//======================================================================================================================

//  little-endian, of course
#define DDS_MAGIC 0x20534444

//  DDS_header.dwFlags
#define DDSD_CAPS                   0x00000001
#define DDSD_HEIGHT                 0x00000002
#define DDSD_WIDTH                  0x00000004
#define DDSD_PITCH                  0x00000008
#define DDSD_PIXELFORMAT            0x00001000
#define DDSD_MIPMAPCOUNT            0x00020000
#define DDSD_LINEARSIZE             0x00080000
#define DDSD_DEPTH                  0x00800000

//  DDS_header.sPixelFormat.dwFlags
#define DDPF_ALPHAPIXELS            0x00000001
#define DDPF_FOURCC                 0x00000004
#define DDPF_INDEXED                0x00000020
#define DDPF_RGB                    0x00000040

//  DDS_header.sCaps.dwCaps1
#define DDSCAPS_COMPLEX             0x00000008
#define DDSCAPS_TEXTURE             0x00001000
#define DDSCAPS_MIPMAP              0x00400000

//  DDS_header.sCaps.dwCaps2
#define DDSCAPS2_CUBEMAP            0x00000200
#define DDSCAPS2_CUBEMAP_POSITIVEX  0x00000400
#define DDSCAPS2_CUBEMAP_NEGATIVEX  0x00000800
#define DDSCAPS2_CUBEMAP_POSITIVEY  0x00001000
#define DDSCAPS2_CUBEMAP_NEGATIVEY  0x00002000
#define DDSCAPS2_CUBEMAP_POSITIVEZ  0x00004000
#define DDSCAPS2_CUBEMAP_NEGATIVEZ  0x00008000
#define DDSCAPS2_VOLUME             0x00200000

#define D3DFMT_DXT1     '1TXD'    //  DXT1 compression texture format
#define D3DFMT_DXT2     '2TXD'    //  DXT2 compression texture format
#define D3DFMT_DXT3     '3TXD'    //  DXT3 compression texture format
#define D3DFMT_DXT4     '4TXD'    //  DXT4 compression texture format
#define D3DFMT_DXT5     '5TXD'    //  DXT5 compression texture format

#define PF_IS_DXT1(pf) \
  ((pf.dwFlags & DDPF_FOURCC) && \
   (pf.dwFourCC == D3DFMT_DXT1))

#define PF_IS_DXT3(pf) \
  ((pf.dwFlags & DDPF_FOURCC) && \
   (pf.dwFourCC == D3DFMT_DXT3))

#define PF_IS_DXT5(pf) \
  ((pf.dwFlags & DDPF_FOURCC) && \
   (pf.dwFourCC == D3DFMT_DXT5))

#define PF_IS_BGRA8(pf) \
  ((pf.dwFlags & DDPF_RGB) && \
   (pf.dwFlags & DDPF_ALPHAPIXELS) && \
   (pf.dwRGBBitCount == 32) && \
   (pf.dwRBitMask == 0xff0000) && \
   (pf.dwGBitMask == 0xff00) && \
   (pf.dwBBitMask == 0xff) && \
   (pf.dwAlphaBitMask == 0xff000000U))

#define PF_IS_BGR8(pf) \
  ((pf.dwFlags & DDPF_ALPHAPIXELS) && \
  !(pf.dwFlags & DDPF_ALPHAPIXELS) && \
   (pf.dwRGBBitCount == 24) && \
   (pf.dwRBitMask == 0xff0000) && \
   (pf.dwGBitMask == 0xff00) && \
   (pf.dwBBitMask == 0xff))

#define PF_IS_BGR5A1(pf) \
  ((pf.dwFlags & DDPF_RGB) && \
   (pf.dwFlags & DDPF_ALPHAPIXELS) && \
   (pf.dwRGBBitCount == 16) && \
   (pf.dwRBitMask == 0x00007c00) && \
   (pf.dwGBitMask == 0x000003e0) && \
   (pf.dwBBitMask == 0x0000001f) && \
   (pf.dwAlphaBitMask == 0x00008000))

#define PF_IS_BGR565(pf) \
  ((pf.dwFlags & DDPF_RGB) && \
  !(pf.dwFlags & DDPF_ALPHAPIXELS) && \
   (pf.dwRGBBitCount == 16) && \
   (pf.dwRBitMask == 0x0000f800) && \
   (pf.dwGBitMask == 0x000007e0) && \
   (pf.dwBBitMask == 0x0000001f))

#define PF_IS_INDEX8(pf) \
  ((pf.dwFlags & DDPF_INDEXED) && \
   (pf.dwRGBBitCount == 8))


union DDS_header
{
	struct
	{
		uint dwMagic;
		uint dwSize;
		uint dwFlags;
		uint dwHeight;
		uint dwWidth;
		uint dwPitchOrLinearSize;
		uint dwDepth;
		uint dwMipMapCount;
		uint dwReserved1[11];

		//  DDPIXELFORMAT
		struct
		{
			uint dwSize;
			uint dwFlags;
			uint dwFourCC;
			uint dwRGBBitCount;
			uint dwRBitMask;
			uint dwGBitMask;
			uint dwBBitMask;
			uint dwAlphaBitMask;
		} sPixelFormat;

		//  DDCAPS2
		struct
		{
			uint dwCaps1;
			uint dwCaps2;
			uint dwDDSX;
			uint dwReserved;
		} sCaps;
		uint dwReserved2;
	} data;
	char dataArr[128];
};

struct DdsLoadInfo
{
	bool compressed;
	bool swap;
	bool palette;
	unsigned int divSize;
	unsigned int blockBytes;
};

DdsLoadInfo loadInfoDXT1 = {true, false, false, 4, 8};
DdsLoadInfo loadInfoDXT3 = {true, false, false, 4, 16};
DdsLoadInfo loadInfoDXT5 = {true, false, false, 4, 16};
DdsLoadInfo loadInfoBGRA8 = {false, false, false, 1, 4};
DdsLoadInfo loadInfoBGR8 = {false, false, false, 1, 3};
DdsLoadInfo loadInfoBGR5A1 = {false, true, false, 1, 2};
DdsLoadInfo loadInfoBGR565 = {false, true, false, 1, 2};
DdsLoadInfo loadInfoIndex8 = {false, false, true, 1, 1};

void Image::loadDds(const char* filename)
{
	std::fstream in;
	in.open(filename, std::ios::in | std::ios::binary);

	if(!in.is_open())
	{
		throw EXCEPTION("Cannot open file");
	}

	//
	// Read header
	//
	DDS_header hdr;
	in.read((char*)&hdr, sizeof(hdr));

	if(hdr.data.dwMagic != DDS_MAGIC || hdr.data.dwSize != 124 || !(hdr.data.dwFlags & DDSD_PIXELFORMAT) ||
	   !(hdr.data.dwFlags & DDSD_CAPS))
	{
		throw EXCEPTION("Incorrect DDS header");
	}

	//
	// Determine the format
	//
	DdsLoadInfo * li;

	if(PF_IS_DXT1(hdr.data.sPixelFormat))
	{
		li = &loadInfoDXT1;
		dataCompression = DC_DXT1;
	}
	else if(PF_IS_DXT3(hdr.data.sPixelFormat))
	{
		li = &loadInfoDXT3;
		dataCompression = DC_DXT3;
	}
	else if(PF_IS_DXT5(hdr.data.sPixelFormat))
	{
		li = &loadInfoDXT5;
		dataCompression = DC_DXT5;
	}
	else
	{
		throw EXCEPTION("Unsupported DDS format");
	}

	//
	// Load the data
	//
	uint mipMapCount = (hdr.data.dwFlags & DDSD_MIPMAPCOUNT) ? hdr.data.dwMipMapCount : 1;
	if(mipMapCount != 1)
	{
		throw EXCEPTION("Currently mipmaps are not supported in DDS");
	}

	uint x = hdr.data.dwWidth;
	uint y = hdr.data.dwHeight;

	if(li->compressed)
	{
		size_t size = std::max(li->divSize, x) / li->divSize * std::max(li->divSize, y) / li->divSize * li->blockBytes;
		/*assert( size == hdr.dwPitchOrLinearSize );
		assert( hdr.dwFlags & DDSD_LINEARSIZE );*/
		data.resize(size);
		in.read((char*)(&data[0]), size);
	}

	type = CT_RGBA;
	width = hdr.data.dwWidth;
	height = hdr.data.dwHeight;

	in.close();
}
