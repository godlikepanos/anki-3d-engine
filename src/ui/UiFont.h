#ifndef UI_FONT_H
#define UI_FONT_H

#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include "util/StdTypes.h"
#include "math/Math.h"


class Texture;


/// The font is device agnostic, all sizes are in actual pixels
class UiFont
{
	public:
		/// Constructor @see create
		UiFont(const char* fontFilename, uint nominalWidth, uint nominalHeight);

		/// @name Accessors
		/// @{
		const Mat3& getGlyphTextureMatrix(char c) const;
		uint getGlyphWidth(char c) const;
		uint getGlyphHeight(char c) const;
		int getGlyphAdvance(char c) const;
		int getGlyphBearingX(char c) const;
		int getGlyphBearingY(char c) const;

		const Texture& getMap() const
		{
			return *map;
		}

		uint getLineHeight() const
		{
			return lineHeight;
		}
		/// @}

	private:
		struct Glyph
		{
			/// Transforms the default texture coordinates to the glyph's. The
			/// default are (1, 1), (0, 1), (0, 0), (0, 1)
			Mat3 textureMat;
			uint width;
			uint height;
			int horizBearingX;
			int horizBearingY;
			int horizAdvance;
		};

		/// The texture map that contains all the glyphs
		boost::scoped_ptr<Texture> map;
		/// A set of glyphs from ' ' to ' ' + 128
		boost::ptr_vector<Glyph> glyphs;
		uint lineHeight;

		/// @param[in] fontFilename The filename of the font to load
		/// @param[in] nominalWidth The nominal glyph width in pixels
		/// @param[in] nominalHeight The nominal glyph height in pixels
		void create(const char* fontFilename, uint nominalWidth,
			uint nominalHeight);
};


inline UiFont::UiFont(const char* fontFilename, uint nominalWidth,
	uint nominalHeight)
{
	create(fontFilename, nominalWidth, nominalHeight);
}


inline const Mat3& UiFont::getGlyphTextureMatrix(char c) const
{
	return glyphs[c - ' '].textureMat;
}


inline uint UiFont::getGlyphWidth(char c) const
{
	return glyphs[c - ' '].width;
}


inline uint UiFont::getGlyphHeight(char c) const
{
	return glyphs[c - ' '].height;
}


inline int UiFont::getGlyphAdvance(char c) const
{
	return glyphs[c - ' '].horizAdvance;
}


inline int UiFont::getGlyphBearingX(char c) const
{
	return glyphs[c - ' '].horizBearingX;
}


inline int UiFont::getGlyphBearingY(char c) const
{
	return glyphs[c - ' '].horizBearingY;
}


#endif
