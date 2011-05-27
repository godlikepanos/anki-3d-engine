#ifndef UI_FONT_H
#define UI_FONT_H

#include <memory>
#include <boost/ptr_container/ptr_vector.hpp>
#include "StdTypes.h"
#include "Math.h"


class Texture;


namespace Ui {


/// The font is device agnostic, all sizes are in actual pixels
class Font
{
	public:
		/// Constructor @see create
		Font(const char* fontFilename, uint nominalWidth, uint nominalHeight);

		/// @name Accessors
		/// @{
		const Mat3& getGlyphTextureMatrix(char c) const {return glyphs[c - ' '].textureMat;}
		uint getGlyphWidth(char c) const {return glyphs[c - ' '].width;}
		uint getGlyphHeight(char c) const {return glyphs[c - ' '].height;}
		/// @}

	private:
		struct Glyph
		{
			/// Transforms the default texture coordinates to the glyph's. The default are (1, 1), (0, 1), (0, 0),
			/// (0, 1)
			Mat3 textureMat;
			uint width;
			uint height;
			int horizBearingX;
			int horizBearingY;
			int horizAdvance;
		};

		std::auto_ptr<Texture> map; ///< The texture map that contains all the glyphs
		boost::ptr_vector<Glyph> glyphs; ///< A set of glyphs from ' ' to ' ' + 128

		/// @param[in] fontFilename The filename of the font to load
		/// @param[in] nominalWidth The nominal glyph width in pixels
		/// @param[in] nominalHeight The nominal glyph height in pixels
		void create(const char* fontFilename, uint nominalWidth, uint NominalHeight);
};


inline Font::Font(const char* fontFilename, uint nominalWidth, uint nominalHeight)
{
	create(fontFilename, nominalWidth, nominalHeight);
}


} // end namespace


#endif
