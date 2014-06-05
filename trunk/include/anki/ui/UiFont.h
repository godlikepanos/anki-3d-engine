// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_UI_UI_FONT_H
#define ANKI_UI_UI_FONT_H

#include <boost/ptr_container/ptr_vector.hpp>
#include "anki/Math.h"

namespace anki {

class Texture;

/// The font is device agnostic, all sizes are in actual pixels
class UiFont
{
public:
	/// Constructor @see create
	UiFont(const char* fontFilename, uint nominalWidth, uint nominalHeight)
	{
		create(fontFilename, nominalWidth, nominalHeight);
	}

	/// @name Accessors
	/// @{
	const Mat3& getGlyphTextureMatrix(char c) const
	{
		return glyphs[c - ' '].textureMat;
	}

	uint32_t getGlyphWidth(char c) const
	{
		return glyphs[c - ' '].width;
	}

	uint32_t getGlyphHeight(char c) const
	{
		return glyphs[c - ' '].height;
	}

	int getGlyphAdvance(char c) const
	{
		return glyphs[c - ' '].horizAdvance;
	}

	int getGlyphBearingX(char c) const
	{
		return glyphs[c - ' '].horizBearingX;
	}

	int getGlyphBearingY(char c) const
	{
		return glyphs[c - ' '].horizBearingY;
	}

	const Texture& getMap() const
	{
		return *map;
	}

	uint32_t getLineHeight() const
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
		uint32_t width;
		uint32_t height;
		int horizBearingX;
		int horizBearingY;
		int horizAdvance;
	};

	/// The texture map that contains all the glyphs
	std::unique_ptr<Texture> map;
	/// A set of glyphs from ' ' to ' ' + 128
	boost::ptr_vector<Glyph> glyphs;
	uint32_t lineHeight;

	/// @param[in] fontFilename The filename of the font to load
	/// @param[in] nominalWidth The nominal glyph width in pixels
	/// @param[in] nominalHeight The nominal glyph height in pixels
	void create(const char* fontFilename, uint32_t nominalWidth,
		uint32_t nominalHeight);
};

} // end namespace anki

#endif
