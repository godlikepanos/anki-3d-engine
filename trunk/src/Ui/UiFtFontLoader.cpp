#include <boost/foreach.hpp>
#include "UiFtFontLoader.h"
#include "Exception.h"


namespace Ui {


//======================================================================================================================
// getGlyphs                                                                                                           =
//======================================================================================================================
void FtFontLoader::getGlyphs()
{
	glyphs.resize(GLYPHS_NUM);

	for(uint n = 1; n < GLYPHS_NUM; n++)
	{
		char c = ' ' + n;

		FT_UInt glyphIndex = FT_Get_Char_Index(face, c);

		FT_Error error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
		if(error)
		{
			throw EXCEPTION("FT_Load_Glyph failed");
		}

		glyphs[n].metrics = face->glyph->metrics;

		error = FT_Get_Glyph(face->glyph, &glyphs[n].glyph);
		if(error)
		{
			throw EXCEPTION("FT_Get_Glyph failed");
		}
	}

	glyphs[0].metrics.width = glyphs['_' - ' '].metrics.width;
}


//======================================================================================================================
// copyBitmap                                                                                                          =
//======================================================================================================================
void FtFontLoader::copyBitmap(const uchar* srcImg, const FT_Vector& srcSize, const FT_Vector& pos)
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


//======================================================================================================================
// computeImageSize                                                                                                    =
//======================================================================================================================
void FtFontLoader::computeImageSize()
{
	imgSize.x = 0;
	imgSize.y = 0;

	//
	// Get img height
	//
	BOOST_FOREACH(const Glyph& glyph, glyphs)
	{
		if(toPixels(glyph.metrics.height) > imgSize.y)
		{
			imgSize.y = toPixels(glyph.metrics.height);
		}
	}

	imgSize.y = GLYPHS_NUM / GLYPHS_PER_ROW * imgSize.y;

	//
	// Get img width
	//


	// For all rows
	for(uint i = 0; i < GLYPHS_NUM / GLYPHS_PER_ROW; i++)
	{
		uint rowSize = 0;
		// For all columns
		for(uint j = 0; j < GLYPHS_PER_ROW; j++)
		{
			rowSize += toPixels(glyphs[i * GLYPHS_PER_ROW + j].metrics.width);
		}

		if(rowSize > imgSize.x)
		{
			imgSize.x = rowSize;
		}
	}
}



//======================================================================================================================
// createImage                                                                                                         =
//======================================================================================================================
void FtFontLoader::createImage(const char* filename, const FT_Vector& fontSize)
{
	FT_Error error;

	// Create lib
	error = FT_Init_FreeType(&library);
	if(error)
	{
		throw EXCEPTION("FT_Init_FreeType failed");
	}

	// Create face and set glyph size
	error = FT_New_Face(library, filename, 0, &face);
	if(error)
	{
		throw EXCEPTION("FT_New_Face failed with filename \"" + filename + "\"");
	}

	FT_Set_Pixel_Sizes(face, fontSize.x, fontSize.y);

	// Get all glyphs
	getGlyphs();

	// Get final image size and create image buffer
	computeImageSize();

	size_t size = imgSize.x * imgSize.y * 2 * sizeof(uchar);
	img.resize(size, 128);

	// Draw all glyphs to the image
	FT_Vector pos = {0, 0};
	// For all rows
	for(uint i = 0; i < GLYPHS_NUM / GLYPHS_PER_ROW; i++)
	{
		// For all columns
		for(uint j = 0; j < GLYPHS_PER_ROW; j++)
		{
			Glyph& glyph = glyphs[i * GLYPHS_PER_ROW + j];

			FT_Glyph_To_Bitmap(&glyph.glyph, FT_RENDER_MODE_NORMAL, 0, 0);
			FT_BitmapGlyph bit = (FT_BitmapGlyph) glyph.glyph;

			FT_Vector srcSize = {toPixels(glyph.metrics.width), toPixels(glyph.metrics.height)};

			copyBitmap(bit->bitmap.buffer, srcSize, pos);

			pos.x += toPixels(glyph.metrics.width);
		}

		pos.y += imgSize.y / (GLYPHS_NUM / GLYPHS_PER_ROW);
	}

	// Clean
	BOOST_FOREACH(Glyph& glyph, glyphs)
	{
		FT_Done_Glyph(glyph.glyph);
	}

	FT_Done_Face(face);
	FT_Done_FreeType(library);
}


//======================================================================================================================
// saveImage                                                                                                           =
//======================================================================================================================
void FtFontLoader::saveImage(const char* filename) const
{
	char tgaHeader[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	FILE* fp = fopen(filename, "wb");

	fwrite(tgaHeader, 1, sizeof(tgaHeader), fp);

	uchar header6[6];

	header6[0] = imgSize.x % 256;
	header6[1] = imgSize.x / 256;
	header6[2] = imgSize.y % 256;
	header6[3] = imgSize.y / 256;
	header6[4] = 24;
	header6[5] = 0;

	fwrite(header6, 1, sizeof(header6), fp);

	for(int i = imgSize.y - 1; i > -1; i--)
	{
		for(int j = 0; j < imgSize.x; j++)
		{
			fwrite(&img[i * imgSize.x + j], 1, sizeof(uchar), fp);
			fwrite(&img[i * imgSize.x + j], 1, sizeof(uchar), fp);
			fwrite(&img[i * imgSize.x + j], 1, sizeof(uchar), fp);
		}
	}

	fclose(fp);
}


} // end namespace
