#include <GL/glew.h>
#include <boost/foreach.hpp>
#include "UiFont.h"
#include "Texture.h"
#include "Exception.h"
#include "Assert.h"
#include "UiFtFontLoader.h"


namespace Ui {


//======================================================================================================================
// create                                                                                                              =
//======================================================================================================================
void Font::create(const char* fontFilename, uint nominalWidth, uint NominalHeight)
{
	FT_Vector ftSize = {nominalWidth, NominalHeight};
	FtFontLoader ft(fontFilename, ftSize);

	ft.saveImage("/tmp/test.tga");

	// - Create glyphs
	// - Get metrics
	BOOST_FOREACH(const FtFontLoader::Glyph& ftGlyph, ft.getGlyphs())
	{
		// Create
		glyphs.push_back(new Glyph);
		Glyph& glyph = glyphs.back();

		// Metrics
		glyph.width = FtFontLoader::toPixels(ftGlyph.getMetrics().width);
		glyph.height = FtFontLoader::toPixels(ftGlyph.getMetrics().height);
		glyph.horizBearingX = FtFontLoader::toPixels(ftGlyph.getMetrics().horiBearingX);
		glyph.horizBearingY = FtFontLoader::toPixels(ftGlyph.getMetrics().horiBearingY);
		glyph.horizAdvance = FtFontLoader::toPixels(ftGlyph.getMetrics().horiAdvance);
	}

	//
	// Calc texture matrices
	//
	int posX = 0;
	int posY = 0;

	// For all rows
	for(uint i = 0; i < FtFontLoader::GLYPH_ROWS; i++)
	{
		// For all columns
		for(uint j = 0; j < FtFontLoader::GLYPH_COLUMNS; j++)
		{
			Glyph& glyph = glyphs[i * FtFontLoader::GLYPH_COLUMNS + j];

			// Set texture matrix
			float scaleX = glyph.width / float(ft.getImageSize().x); // glyph width
			float scaleY = glyph.height / float(ft.getImageSize().y); // glyph height
			float tslX = posX / float(ft.getImageSize().x);
			float tslY = (ft.getImageSize().y - glyph.height - posY) / float(ft.getImageSize().y);

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


} // end namespace
