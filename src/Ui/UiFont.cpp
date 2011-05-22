#include <boost/foreach.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include "UiFont.h"
#include "Vec.h"
#include "Texture.h"
#include "Exception.h"
#include "Assert.h"


struct Glyph
{
	FT_Glyph glyph;
	FT_Glyph_Metrics metrics;
};


inline FT_Int toPixels(FT_Int a)
{
	return a >> 6;
}


static void getGlyphs(FT_Face& face, Vec<Glyph>& glyphs)
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


static void copyBitmap(const unsigned char* srcImg, const FT_Vector& srcSize, const FT_Vector& pos,
                       Vec<uchar>& destImg, const FT_Vector& destSize)
{
	for(int i = 0; i < srcSize.y; i++)
	{
		for(int j = 0; j < srcSize.x; j++)
		{
			int jj = j + pos.x;
			ASSERT(jj < destSize.x);
			int ii = i + pos.y;
			ASSERT(ii < destSize.y);
			destImg[ii * destSize.x + jj] = srcImg[i * srcSize.x + j];
		}
	}
}


static void computeImageSize(const Vec<Glyph>& glyphs, FT_Vector& size)
{
	size.x = 0;
	size.y = 0;

	BOOST_FOREACH(const Glyph& glyph, glyphs)
	{
		size.x += glyph.metrics.width >> 6;
		if(size.y < glyph.metrics.height >> 6)
		{
			size.y = glyph.metrics.height >> 6;
		}
	}
}


static void createImage(Vec<Glyph>& glyphs, Vec<uchar>& img)
{
	FT_Vector imgSize;
	computeImageSize(glyphs, imgSize);

	size_t size = imgSize.x * imgSize.y * 2 * sizeof(uchar);
	img.resize(size, 128);

	FT_Vector pos = {0, 0};

	BOOST_FOREACH(Glyph& glyph, glyphs)
	{
		FT_Glyph_To_Bitmap(&glyph.glyph, FT_RENDER_MODE_NORMAL, 0, 0);
		FT_BitmapGlyph bit = (FT_BitmapGlyph) glyph.glyph;

		FT_Vector srcSize = {toPixels(glyph.metrics.width), toPixels(glyph.metrics.height)};

		copyBitmap(bit->bitmap.buffer, srcSize, pos, img, imgSize);

		pos.x += toPixels(glyph.metrics.width);
	}

	//save_image(image, imgSize.x, imgSize.y);
}
