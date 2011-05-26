#ifndef UI_FT_FONT_LOADER_H
#define UI_FT_FONT_LOADER_H

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include "Vec.h"
#include "StdTypes.h"
#include "Math.h"


namespace Ui {


/// A helper class that uses libfreetype to load glyphs from a font file and gather the metrics for each glyhp
class FtFontLoader
{
	public:
		enum
		{
			GLYPHS_NUM = 128,
			GLYPH_COLUMNS = 16,
			GLYPH_ROWS = 8
		};

		FtFontLoader(const char* filename, const FT_Vector& fontSize);

		/// Save the image (img) to TGA. Its for debugging purposes
		void saveImage(const char* filename) const;

	private:
		struct Glyph
		{
			FT_Glyph glyph;
			FT_Glyph_Metrics metrics;
			Mat3 textureMat;
		};

		/// @name Data
		/// @{
		FT_Library library;
		FT_Face face;
		Vec<Glyph> glyphs;
		Vec<uchar> img;
		FT_Vector imgSize;
		/// @}

		FT_Int toPixels(FT_Int a) {return a >> 6;}

		/// Reads the face and extracts the glyphs
		void getGlyphs();

		/// Copy one bitmap to img
		void copyBitmap(const uchar* srcImg, const FT_Vector& srcSize, const FT_Vector& pos);

		/// Compute image size (imgSize) using the glyphs set
		void computeImageSize();

		/// Given a filename and a font size create an image with all the glyphs
		void createImage(const char* filename, const FT_Vector& fontSize);
};


inline FtFontLoader::FtFontLoader(const char* filename, const FT_Vector& fontSize)
{
	createImage(filename, fontSize);
}


} // end namespace


#endif
