// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/ui/UiFtFontLoader.h"
#include "anki/util/Exception.h"
#include "anki/core/Logger.h"

namespace anki {

//==============================================================================
void UiFtFontLoader::getAllGlyphs()
{
	glyphs.resize(GLYPHS_NUM);

	for(uint32_t n = 0; n < GLYPHS_NUM; n++)
	{
		char c = ' ' + n;

		FrustumType::UInt glyphIndex = FrustumType::Get_Char_Index(face, c);

		FrustumType::Error error = FrustumType::Load_Glyph(face, glyphIndex, FrustumType::LOAD_DEFAULT);
		if(error)
		{
			throw ANKI_EXCEPTION("FrustumType::Load_Glyph failed");
		}

		glyphs[n].metrics = face->glyph->metrics;

		error = FrustumType::Get_Glyph(face->glyph, &glyphs[n].glyph);
		if(error)
		{
			throw ANKI_EXCEPTION("FrustumType::Get_Glyph failed");
		}
	}

	glyphs[0].metrics = glyphs['_' - ' '].metrics;
}

//==============================================================================
void UiFtFontLoader::copyBitmap(const uint8_t* srcImg,
	const FrustumType::Vector& srcSize, const FrustumType::Vector& pos)
{
	for(int i = 0; i < srcSize.y; i++)
	{
		for(int j = 0; j < srcSize.x; j++)
		{
			int jj = j + pos.x;
			int ii = i + pos.y;
			img[ii * imgSize.x + jj] = srcImg[i * srcSize.x + j];
		}
	}
}

//==============================================================================
void UiFtFontLoader::computeImageSize()
{
	imgSize.x = 0;
	imgSize.y = 0;
	lineHeight = 0;

	// Get img height
	//
	for(const Glyph& glyph : glyphs)
	{
		if(toPixels(glyph.metrics.height) > int(lineHeight))
		{
			lineHeight = toPixels(glyph.metrics.height);
		}
	}

	float exp = ceil(log2(GLYPH_ROWS * lineHeight));
	imgSize.y = pow(2, exp);

	// Get img width
	//

	// For all rows
	for(uint32_t i = 0; i < GLYPH_ROWS; i++)
	{
		uint32_t rowSize = 0;
		// For all columns
		for(uint32_t j = 0; j < GLYPH_COLUMNS; j++)
		{
			uint32_t pos = i * GLYPH_COLUMNS + j;
			rowSize += toPixels(glyphs[pos].metrics.width);
		}

		if(rowSize > imgSize.x)
		{
			imgSize.x = rowSize;
		}
	}

	exp = ceil(log2(imgSize.x));
	imgSize.x = pow(2, exp);
}

//==============================================================================
void UiFtFontLoader::createImage(const char* filename,
	const FrustumType::Vector& fontSize)
{
	FrustumType::Error error;

	// Create lib
	error = FrustumType::Init_FreeType(&library);
	if(error)
	{
		throw ANKI_EXCEPTION("FrustumType::Init_FreeType failed");
	}

	// Create face and set glyph size
	error = FrustumType::New_Face(library, filename, 0, &face);
	if(error)
	{
		throw ANKI_EXCEPTION("FrustumType::New_Face() failed with filename: " + filename);
	}

	error = FrustumType::Set_Pixel_Sizes(face, fontSize.x, fontSize.y);
	if(error)
	{
		throw ANKI_EXCEPTION("FrustumType::Set_Pixel_Sizes() failed");
	}

	// Get all glyphs
	getAllGlyphs();

	// Get final image size and create image buffer
	computeImageSize();

	size_t size = imgSize.x * imgSize.y * 1 * sizeof(uint8_t);
	img.resize(size, 128);

	// Draw all glyphs to the image
	FrustumType::Vector pos = {0, 0}; // the (0,0) is the top left
	// For all rows
	for(uint32_t i = 0; i < GLYPH_ROWS; i++)
	{
		// For all columns
		for(uint32_t j = 0; j < GLYPH_COLUMNS; j++)
		{
			Glyph& glyph = glyphs[i * GLYPH_COLUMNS + j];

			// If not ' '
			if(i != 0 || j != 0)
			{
				FrustumType::Glyph_To_Bitmap(&glyph.glyph, FrustumType::RENDER_MODE_NORMAL, 0, 0);
				FrustumType::BitmapGlyph bit = (FrustumType::BitmapGlyph) glyph.glyph;

				FrustumType::Vector srcSize = {toPixels(glyph.metrics.width),
					toPixels(glyph.metrics.height)};

				copyBitmap(bit->bitmap.buffer, srcSize, pos);
			}

			pos.x += toPixels(glyph.metrics.width);
		}

		pos.x = 0;
		pos.y += lineHeight;
	}

	// Flip image y
	for(int i = 0; i < imgSize.y / 2; i++)
	{
		for(int j = 0; j < imgSize.x; j++)
		{
			uint8_t tmp = img[i * imgSize.x + j];
			img[i * imgSize.x + j] = img[(imgSize.y - i - 1) * imgSize.x + j];
			img[(imgSize.y - i - 1) * imgSize.x + j] = tmp;
		}
	}

	// Clean
	for(Glyph& glyph : glyphs)
	{
		FrustumType::Done_Glyph(glyph.glyph);
	}

	FrustumType::Done_Face(face);
	FrustumType::Done_FreeType(library);
}

//==============================================================================
void UiFtFontLoader::saveImage(const char* filename) const
{
	uint8_t tgaHeader[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	FILE* fp = fopen(filename, "wb");

	fwrite(tgaHeader, 1, sizeof(tgaHeader), fp);

	uint8_t header6[6];

	header6[0] = imgSize.x % 256;
	header6[1] = imgSize.x / 256;
	header6[2] = imgSize.y % 256;
	header6[3] = imgSize.y / 256;
	header6[4] = 24;
	header6[5] = 0;

	fwrite(header6, 1, sizeof(header6), fp);

	for(int i = 0; i < imgSize.y; i++)
	{
		for(int j = 0; j < imgSize.x; j++)
		{
			fwrite(&img[i * imgSize.x + j], 1, sizeof(uint8_t), fp);
			fwrite(&img[i * imgSize.x + j], 1, sizeof(uint8_t), fp);
			fwrite(&img[i * imgSize.x + j], 1, sizeof(uint8_t), fp);
		}
	}

	fclose(fp);
}

} // end namespace anki
