#include "UiFont.h"
#include "resource/Texture.h"
#include "util/Exception.h"
#include "util/Assert.h"
#include "UiFtFontLoader.h"
#include <GL/glew.h>
#include <boost/foreach.hpp>


//==============================================================================
// create                                                                      =
//==============================================================================
void UiFont::create(const char* fontFilename, uint nominalWidth,
	uint NominalHeight)
{
	FT_Vector ftSize = {nominalWidth, NominalHeight};
	UiFtFontLoader ft(fontFilename, ftSize);

	lineHeight = ft.getLineHeight();

	//ft.saveImage("/tmp/test.tga");

	// - Create glyphs
	// - Get metrics
	BOOST_FOREACH(const UiFtFontLoader::Glyph& ftGlyph, ft.getGlyphs())
	{
		// Create
		glyphs.push_back(new Glyph);
		Glyph& glyph = glyphs.back();

		// Metrics
		glyph.width = UiFtFontLoader::toPixels(ftGlyph.getMetrics().width);
		glyph.height = UiFtFontLoader::toPixels(ftGlyph.getMetrics().height);
		glyph.horizBearingX = UiFtFontLoader::toPixels(
			ftGlyph.getMetrics().horiBearingX);
		glyph.horizBearingY = UiFtFontLoader::toPixels(
			ftGlyph.getMetrics().horiBearingY);
		glyph.horizAdvance = UiFtFontLoader::toPixels(
			ftGlyph.getMetrics().horiAdvance);
	}

	//
	// Calc texture matrices
	//
	int posX = 0;
	int posY = 0;

	// For all rows
	for(uint i = 0; i < UiFtFontLoader::GLYPH_ROWS; i++)
	{
		// For all columns
		for(uint j = 0; j < UiFtFontLoader::GLYPH_COLUMNS; j++)
		{
			Glyph& glyph = glyphs[i * UiFtFontLoader::GLYPH_COLUMNS + j];

			// Set texture matrix
			// glyph width
			float scaleX = glyph.width / float(ft.getImageSize().x);
			// glyph height
			float scaleY = glyph.height / float(ft.getImageSize().y);
			float tslX = posX / float(ft.getImageSize().x);
			float tslY = (ft.getImageSize().y - glyph.height - posY) /
				float(ft.getImageSize().y);

			glyph.textureMat = Mat3::getIdentity();
			glyph.textureMat(0, 0) = scaleX;
			glyph.textureMat(1, 1) = scaleY;
			glyph.textureMat(0, 2) = tslX;
			glyph.textureMat(1, 2) = tslY;

			//std::cout << glyph.textureMat << std::endl;

			posX += glyph.width;
		}

		posX = 0;
		posY += ft.getLineHeight();
	}

	//
	// Create the texture
	//
	Texture::Initializer tinit;
	tinit.width = ft.getImageSize().x;
	tinit.height = ft.getImageSize().y;
	tinit.internalFormat = GL_RED;
	tinit.format = GL_RED;
	tinit.type = GL_UNSIGNED_BYTE;
	tinit.data = &ft.getImage()[0];
	tinit.mipmapping = false;
	tinit.filteringType = Texture::TFT_NEAREST;
	tinit.anisotropyLevel = 0;
	tinit.dataCompression = Texture::DC_NONE;

	map.reset(new Texture());
	map->create(tinit);
	map->setRepeat(false);
}
