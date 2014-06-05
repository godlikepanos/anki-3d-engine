// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/ui/UiPainter.h"
#include "anki/gl/GlState.h"
#include "anki/resource/TextureResource.h"
#include "anki/core/Logger.h"
#include "anki/ui/UiFont.h"
#include <cstdarg>
#include <cstdio>

namespace anki {

//==============================================================================
UiPainter::UiPainter(const Vec2& deviceSize_)
	: deviceSize(deviceSize_)
{
	init();
}

//==============================================================================
void UiPainter::setFont(const char* fontFilename, uint nominalWidth,
	uint nominalHeight)
{
	font.reset(new UiFont(fontFilename, nominalWidth, nominalHeight));
}

//==============================================================================
void UiPainter::init()
{
	// Default font
	float dfltFontWidth = 0.2 * deviceSize.x();
	font.reset(new UiFont("engine-rsrc/ConsolaMono.ttf", dfltFontWidth,
		deviceSize.x() / deviceSize.y() * dfltFontWidth));

	// Misc
	tabSize = 4;

	// SProg
	sProg.load("shaders/UiText.glsl");
	sProg->bind();
	sProg->findUniformVariable("texture").set(font->getMap());
	sProg->findUniformVariable("color").set(col);

	// Geom
	float quadVertCoords[][2] = {{1.0, 1.0}, {0.0, 1.0}, {0.0, 0.0},
		{1.0, 0.0}};
	qPositionsVbo.create(GL_ARRAY_BUFFER, sizeof(quadVertCoords),
		quadVertCoords, GL_STATIC_DRAW);

	ushort quadVertIndeces[2][3] = {{0, 1, 3}, {1, 2, 3}}; // 2 triangles
	qIndecesVbo.create(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadVertIndeces),
		quadVertIndeces, GL_STATIC_DRAW);

	qVao.create();
	qVao.attachArrayBufferVbo(qPositionsVbo, 0, 2, GL_FLOAT, false, 0, NULL);
	qVao.attachElementArrayBufferVbo(qIndecesVbo);
}

//==============================================================================
void UiPainter::drawText(const char* text)
{
	// Set GL
	GlStateSingleton::get().enable(GL_BLEND);
	//GlStateMachineSingleton::get().disable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GlStateSingleton::get().disable(GL_DEPTH_TEST);

	// SProg (some)
	sProg->bind();
	sProg->findUniformVariable("texture").set(font->getMap());

	// Vao
	qVao.bind();

	// Iterate
	Vec2 p; // in NDC
	p.x() = 2.0 * pos.x() - 1.0;
	p.y() = 2.0 * (1.0 - pos.y()) - 1.0;
	const char* c = text;
	while(*c != '\0')
	{
		char cc = *c;

		if(cc == ' ')
		{
			p.x() += 2.0 * font->getGlyphWidth(cc) / deviceSize.x();
		}
		else if(cc == '\n')
		{
			p.x() = 2.0 * pos.x() - 1.0;
			p.y() -= 2.0 * font->getLineHeight() / deviceSize.y();
		}
		else if(cc == '\t') // tab
		{
			p.x() +=  font->getGlyphWidth(' ') * tabSize;
		}
		else if(cc < ' ' || cc > '~') // out of range
		{
			ANKI_LOGE("Char out of range (" << cc << "). Ignoring");
		}
		else
		{
			Mat3 trfM = Mat3::getIdentity();
			trfM(0, 0) = 2.0 * float(font->getGlyphWidth(cc)) / deviceSize.x();
			trfM(1, 1) = 2.0 * float(font->getGlyphHeight(cc)) / deviceSize.y();
			trfM(0, 2) = p.x();
			trfM(1, 2) = p.y() + 2.0 * (font->getGlyphBearingY(cc) -
				int(font->getGlyphHeight(cc))) / deviceSize.y();

			sProg->findUniformVariable("transformation").set(trfM);
			sProg->findUniformVariable("textureTranformation").set(
				font->getGlyphTextureMatrix(cc));

			// Render
			glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_SHORT, 0);

			// Inc
			p.x() += 2.0 * font->getGlyphWidth(cc) / deviceSize.x();
		}

		++c;
	}
}


//==============================================================================
// drawFormatedText                                                            =
//==============================================================================
void UiPainter::drawFormatedText(const char* format, ...)
{
	va_list ap;
	char text[512];

	va_start(ap, format);
	vsprintf(text, format, ap);
	va_end(ap);

	ANKI_ASSERT(strlen(text) < 512);

	drawText(text);
}


} // end namespace
