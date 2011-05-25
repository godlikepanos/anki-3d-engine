#include <boost/foreach.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include "UiFont.h"
#include "Vec.h"
#include "Texture.h"
#include "Exception.h"
#include "Assert.h"


namespace Ui {


struct FtGlyph
{
	FT_Glyph glyph;
	FT_Glyph_Metrics metrics;
};


inline FT_Int toPixels(FT_Int a)
{
	return a >> 6;
}


/// Get 127 glyphs from a face
static void getGlyphs(FT_Face& face, Vec<FtGlyph>& glyphs)
{
	const uint MAX_GLYPHS = 127;
	ASSERT(glyphs.size() == 0);
	glyphs.resize(MAX_GLYPHS);

	for(uint n = 0; n < MAX_GLYPHS; n++)
	{
		char c = '!' + n;

		FT_UInt glyph_index = FT_Get_Char_Index(face, c);

		FT_Error error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
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
}


/// Copy one bitmap to another
static void copyBitmap(const unsigned char* srcImg, const FT_Vector& srcSize, const FT_Vector& pos,
                       Vec<uchar>& destImg, const FT_Vector& destSize)
{
	for(int i = 0; i < srcSize.y; i++)
	{
		for(int j = 0; j < srcSize.x; j++)
		{
			int jj = j + pos.x;
			int ii = i + pos.y;
			destImg[ii * destSize.x + jj] = srcImg[i * srcSize.x + j];
		}
	}
}


/// Compute the size of the image with all the glyphs
static void computeImageSize(const Vec<FtGlyph>& glyphs, FT_Vector& size)
{
	size.x = 0;
	size.y = 0;

	BOOST_FOREACH(const FtGlyph& glyph, glyphs)
	{
		// Inc the width
		size.x += toPixels(glyph.metrics.width);

		// Chose the max height
		if(size.y < toPixels(glyph.metrics.height))
		{
			size.y = toPixels(glyph.metrics.height);
		}
	}
}


/// Given a filename and a font size create an image with all the glyphs
static void createImage(const char* filename, const FT_Vector& fontSize,
                        Vec<FtGlyph>& glyphs, Vec<uchar>& img, FT_Vector& imgSize)
{
	FT_Library library;
	FT_Face face;
	FT_Error error;

	// Create lib
	error = FT_Init_FreeType(&library);
	if(error)
	{
		throw EXCEPTION("FT_Init_FreeType failed");
	}

	// Create face and set glyph size
	error = FT_New_Face(library, filename, 0, &face);
	FT_Set_Pixel_Sizes(face, fontSize.x, fontSize.y);

	// Get all glyphs
	getGlyphs(face, glyphs);

	// Get final image size and create image buffer
	computeImageSize(glyphs, imgSize);

	size_t size = imgSize.x * imgSize.y * 2 * sizeof(uchar);
	img.resize(size, 128);

	// Draw all glyphs to the image
	FT_Vector pos = {0, 0};
	BOOST_FOREACH(FtGlyph& glyph, glyphs)
	{
		FT_Glyph_To_Bitmap(&glyph.glyph, FT_RENDER_MODE_NORMAL, 0, 0);
		FT_BitmapGlyph bit = (FT_BitmapGlyph) glyph.glyph;

		FT_Vector srcSize = {toPixels(glyph.metrics.width), toPixels(glyph.metrics.height)};

		copyBitmap(bit->bitmap.buffer, srcSize, pos, img, imgSize);

		pos.x += toPixels(glyph.metrics.width);
	}

	// Clean
	BOOST_FOREACH(FtGlyph& glyph, glyphs)
	{
		FT_Done_Glyph(glyph.glyph);
	}

	FT_Done_Face(face);
	FT_Done_FreeType(library);

	//save_image(image, imgSize.x, imgSize.y);
}


/// Save image to TGA. For debuging purposes
static void saveImage(const char* filename, const Vec<uchar>& buff, int w, int h)
{
	char tgaHeader[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	FILE* fp = fopen(filename, "wb");

	fwrite(tgaHeader, 1, sizeof(tgaHeader), fp);

	unsigned char header6[6];

	header6[0] = w % 256;
	header6[1] = w / 256;
	header6[2] = h % 256;
	header6[3] = h / 256;
	header6[4] = 24;
	header6[5] = 0;

	fwrite(header6, 1, sizeof(header6), fp);

	for(int i = h - 1; i > -1; i--)
	{
		for(int j = 0; j < w; j++)
		{
			fwrite(&buff[i * w + j], 1, sizeof(unsigned char), fp);
			fwrite(&buff[i * w + j], 1, sizeof(unsigned char), fp);
			fwrite(&buff[i * w + j], 1, sizeof(unsigned char), fp);
		}
	}

	fclose(fp);
}


//======================================================================================================================
// create                                                                                                              =
//======================================================================================================================
void Font::create(const char* fontFilename, uint nominalWidth, uint NominalHeight)
{
	Vec<FtGlyph> glyphs;
	Vec<uchar> img;
	FT_Vector imgSize;
	FT_Vector fontSize = {nominalWidth, NominalHeight};
	createImage(fontFilename, fontSize, glyphs, img, imgSize);

	saveImage("/tmp/test.tga", img, imgSize.x, imgSize.y);
}


} // end namespace
