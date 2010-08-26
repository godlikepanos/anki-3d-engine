#include <png.h>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <fstream>
#include "Image.h"
#include "Util.h"


#define IMG_ERROR(x) ERROR("File \"" << filename << "\": " << x)
#define IMG_WARNING(x) WARNING("File \"" << filename << "\": " << x)


uchar Image::tgaHeaderUncompressed[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uchar Image::tgaHeaderCompressed[12]   = {0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0};


//======================================================================================================================
// loadUncompressedTga                                                                                                 =
//======================================================================================================================
bool Image::loadUncompressedTga(const char* filename, fstream& fs, uint& bpp)
{
	// read the info from header
	uchar header6[6];
	fs.read((char*)&header6[0], sizeof(header6));
	if(fs.gcount() != sizeof(header6))
	{
		IMG_ERROR("Cannot read info header");
		return false;
	}

	width  = header6[1] * 256 + header6[0];
	height = header6[3] * 256 + header6[2];
	bpp = header6[4];

	if((width <= 0) || (height <= 0) || ((bpp != 24) && (bpp !=32)))
	{
		IMG_ERROR("Invalid image information");
		return false;
	}

	// read the data
	int bytesPerPxl	= (bpp / 8);
	int imageSize = bytesPerPxl * width * height;
	data.resize(imageSize);

	fs.read(reinterpret_cast<char*>(&data[0]), imageSize);
	if(fs.gcount() != imageSize)
	{
		IMG_ERROR("Cannot read image data");
		return false;
	}

	// swap red with blue
	for(int i=0; i<int(imageSize); i+=bytesPerPxl)
	{
		uint temp = data[i];
		data[i] = data[i + 2];
		data[i + 2] = temp;
	}

	return true;
}


//======================================================================================================================
// loadCompressedTga                                                                                                   =
//======================================================================================================================
bool Image::loadCompressedTga(const char* filename, fstream& fs, uint& bpp)
{
	unsigned char header6[6];
	fs.read((char*)&header6[0], sizeof(header6));
	if(fs.gcount() != sizeof(header6))
	{
		IMG_ERROR("Cannot read info header");
		return false;
	}

	width  = header6[1] * 256 + header6[0];
	height = header6[3] * 256 + header6[2];
	bpp	= header6[4];

	if((width <= 0) || (height <= 0) || ((bpp != 24) && (bpp !=32)))
	{
		IMG_ERROR("Invalid texture information");
		return false;
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
			IMG_ERROR("Cannot read RLE header");
			return false;
		}

		if(chunkheader < 128)
		{
			chunkheader++;
			for(int counter = 0; counter < chunkheader; counter++)
			{
				fs.read((char*)&colorbuffer[0], bytesPerPxl);
				if(fs.gcount() != bytesPerPxl)
				{
					IMG_ERROR("Cannot read image data");
					return false;
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
					IMG_ERROR("Too many pixels read");
					return false;
				}
			}
		}
		else
		{
			chunkheader -= 127;
			fs.read((char*)&colorbuffer[0], bytesPerPxl);
			if(fs.gcount() != bytesPerPxl)
			{
				IMG_ERROR("Cannot read from file");
				return false;
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
					IMG_ERROR("Too many pixels read");
					return false;
				}
			}
		}
	} while(currentpixel < pixelcount);

	return true;
}


//======================================================================================================================
// loadTga                                                                                                             =
//======================================================================================================================
bool Image::loadTga(const char* filename)
{
	fstream fs;
	char myTgaHeader[12];
	fs.open(filename, ios::in|ios::binary);
	uint bpp;

	if(!fs.good())
	{
		IMG_ERROR("Cannot open file");
		return false;
	}

	fs.read(&myTgaHeader[0], sizeof(myTgaHeader));
	if(fs.gcount() != sizeof(myTgaHeader))
	{
		IMG_ERROR("Cannot read file header");
		fs.close();
		return false;
	}

	bool funcsReturn;
	if(memcmp(tgaHeaderUncompressed, &myTgaHeader[0], sizeof(myTgaHeader)) == 0)
	{
		funcsReturn = loadUncompressedTga(filename, fs, bpp);
	}
	else if(memcmp(tgaHeaderCompressed, &myTgaHeader[0], sizeof(myTgaHeader)) == 0)
	{
		funcsReturn = loadCompressedTga(filename, fs, bpp);
	}
	else
	{
		IMG_ERROR("Invalid image header");
		funcsReturn = false;
	}

	if(bpp == 32)
		type = T_RGBA;
	else
		type = T_RGB;

	fs.close();
	return funcsReturn;
}


//======================================================================================================================
// loadPng                                                                                                             =
//======================================================================================================================
bool Image::loadPng(const char* filename)
{
	/*
	 * Data
	 */
	const uint PNG_SIG_SIZE = 8;
	FILE* file = NULL;
	png_structp pngPtr = NULL;
	png_infop infoPtr = NULL;
	bool ok = false;
	size_t charsRead;
	uint bitDepth;
	uint channels;
	uint rowbytes;
	uint colorType;
	vector<png_bytep> rowPointers;

	/*
	 * Open file
	 */
	file = fopen(filename, "rb");
	if(file == NULL)
	{
		IMG_ERROR("Cannot open file");
		goto cleanup;
	}

	/*
	 * Validate PNG header
	 */
	png_byte pngsig[PNG_SIG_SIZE];
	charsRead = fread(pngsig, 1, PNG_SIG_SIZE, file);
	if(charsRead != PNG_SIG_SIZE)
	{
		IMG_ERROR("Cannot read PNG header");
		goto cleanup;
	}

	if(png_sig_cmp(pngsig, 0, PNG_SIG_SIZE) != 0)
	{
		IMG_ERROR("File not PNG image");
		goto cleanup;
	}

	/*
	 * Crete some PNG structs
	 */
	pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!pngPtr)
	{
		IMG_ERROR("png_create_read_struct failed");
		goto cleanup;
	}

	infoPtr = png_create_info_struct(pngPtr);
	if(!infoPtr)
	{
		IMG_ERROR("png_create_info_struct failed");
		goto cleanup;
	}

	/*
	 * Set error handling
	 */
	if(setjmp(png_jmpbuf(pngPtr)))
	{
		IMG_ERROR("Reading PNG file failed");
		goto cleanup;
	}

	/*
	 * Init io
	 */
	png_init_io(pngPtr, file);
	png_set_sig_bytes(pngPtr, PNG_SIG_SIZE); // PNG lib knows that we allready have read the header

	/*
	 * Read info and make conversions
	 * This loop reads info, if not acceptable it calls libpng funcs to change them and re-runs the loop
	 */
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
				IMG_WARNING("Converting PNG_COLOR_TYPE_PALETTE to PNG_COLOR_TYPE_RGB or PNG_COLOR_TYPE_RGBA");
				png_set_palette_to_rgb(pngPtr);
				goto again;
				break;
			case PNG_COLOR_TYPE_GRAY:
				// do nothing
				break;
			case PNG_COLOR_TYPE_GRAY_ALPHA:
				IMG_WARNING("Cannot accept PNG_COLOR_TYPE_GRAY_ALPHA. Converting to PNG_COLOR_TYPE_GRAY");
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
				FATAL("Forgot to handle a color type");
				break;
		}

		// 2) Convert the bit depths
		if(colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8)
		{
			IMG_WARNING("Converting bit depth");
			png_set_gray_1_2_4_to_8(pngPtr);
			goto again;
		}

		if(bitDepth > 8)
		{
			IMG_WARNING("Converting bit depth");
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
		IMG_ERROR("Sanity checks failed");
		goto cleanup;
	}

	/*
	 * Read this sucker
	 */
	rowbytes = png_get_rowbytes(pngPtr, infoPtr);

	rowPointers.resize(height * sizeof(png_bytep));

	data.resize(rowbytes * height);

	for (uint i = 0; i < height; ++i)
		rowPointers[height - 1 - i] = &data[i * rowbytes];

	png_read_image(pngPtr, &rowPointers[0]);

	/*
	 * Finalize
	 */
	switch(colorType)
	{
		case PNG_COLOR_TYPE_GRAY:
			type = T_R;
			break;
		case PNG_COLOR_TYPE_RGB:
			type = T_RGB;
			break;
		case PNG_COLOR_TYPE_RGBA:
			type = T_RGBA;
			break;
	}

	ok = true;

	/*
	 * Cleanup
	 */
	cleanup:

	if(pngPtr)
	{
		if(infoPtr)
			png_destroy_read_struct(&pngPtr, &infoPtr, NULL);
		else
			png_destroy_read_struct(&pngPtr, NULL, NULL);
	}

	if(file)
		fclose(file);

	return ok;
}


//======================================================================================================================
// load                                                                                                                =
//======================================================================================================================
bool Image::load(const char* filename)
{
	// get the extension
	string ext = filesystem::path(filename).extension();
	to_lower(ext);


	// load from this extension
	if(ext == ".tga")
	{
		if(!loadTga(filename))
		{
			return false;
		}
	}
	else if(ext == ".png")
	{
		if(!loadPng(filename))
		{
			return false;
		}
	}
	else
	{
		IMG_ERROR("Unsupported extension");
		return false;
	}
	return true;
}

