#ifndef UI_FT_FONT_LOADER_H
#define UI_FT_FONT_LOADER_H

#include "Util/Vec.h"
#include "Util/StdTypes.h"
#include "Math/Math.h"
#include "Util/Accessors.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H


namespace Ui {


/// A helper class that uses libfreetype to load glyphs from a font file and
/// gather the metrics for each glyph
class FtFontLoader
{
	public:
		/// Contains info about the glyphs
		class Glyph
		{
			friend class FtFontLoader;

			public:
				GETTER_R(FT_Glyph_Metrics, metrics, getMetrics)

			private:
				FT_Glyph glyph;
				FT_Glyph_Metrics metrics;
		};

		enum
		{
			GLYPHS_NUM = 128,
			GLYPH_COLUMNS = 16,
			GLYPH_ROWS = 8
		};

		/// One and only constructor
		FtFontLoader(const char* filename, const FT_Vector& fontSize);

		/// @name Accessors
		/// @{
		GETTER_R(Vec<uchar>, img, getImage)
		GETTER_R(FT_Vector, imgSize, getImageSize)
		GETTER_R(Vec<Glyph>, glyphs, getGlyphs)
		GETTER_R_BY_VAL(uint, lineHeight, getLineHeight)
		/// @}

		/// Save the image (img) to TGA. Its for debugging purposes
		void saveImage(const char* filename) const;

		static FT_Int toPixels(FT_Int a) {return a >> 6;}

	private:
		/// @name Data
		/// @{
		FT_Library library;
		FT_Face face;
		Vec<Glyph> glyphs;
		Vec<uchar> img;
		FT_Vector imgSize;
		uint lineHeight; ///< Calculated as the max height among all glyphs
		/// @}

		/// Reads the face and extracts the glyphs
		void getAllGlyphs();

		/// Copy one bitmap to img
		void copyBitmap(const uchar* srcImg, const FT_Vector& srcSize,
			const FT_Vector& pos);

		/// Compute image size (imgSize) using the glyphs set
		void computeImageSize();

		/// Given a filename and a font size create an image with all the glyphs
		void createImage(const char* filename, const FT_Vector& fontSize);
};


inline FtFontLoader::FtFontLoader(const char* filename,
	const FT_Vector& fontSize)
{
	createImage(filename, fontSize);
}


} // end namespace


#endif
