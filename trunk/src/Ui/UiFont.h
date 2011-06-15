#ifndef UI_FONT_H
#define UI_FONT_H

#include <boost/scoped_ptr.hpp>
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
		int getGlyphAdvance(char c) const {return glyphs[c - ' '].horizAdvance;}
		int getGlyphBearingX(char c) const {return glyphs[c - ' '].horizBearingX;}
		int getGlyphBearingY(char c) const {return glyphs[c - ' '].horizBearingY;}
		const Texture& getMap() const {return *map;}
		GETTER_R_BY_VAL(uint, lineHeight, getLineHeight)
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

		boost::scoped_ptr<Texture> map; ///< The texture map that contains all the glyphs
		boost::ptr_vector<Glyph> glyphs; ///< A set of glyphs from ' ' to ' ' + 128
		uint lineHeight;

		/// @param[in] fontFilename The filename of the font to load
		/// @param[in] nominalWidth The nominal glyph width in pixels
		/// @param[in] nominalHeight The nominal glyph height in pixels
		void create(const char* fontFilename, uint nominalWidth, uint nominalHeight);
};


inline Font::Font(const char* fontFilename, uint nominalWidth, uint nominalHeight)
{
	create(fontFilename, nominalWidth, nominalHeight);
}


} // end namespace


#endif
