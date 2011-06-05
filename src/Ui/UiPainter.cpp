#include <cstdarg>
#include "UiPainter.h"
#include "GlStateMachine.h"
#include "Texture.h"
#include "Logger.h"
#include "UiFont.h"


namespace Ui {


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
Painter::Painter(uint deviceWidth, uint deviceHeight):
	deviceWidth(deviceWidth),
	deviceHeight(deviceHeight)
{
	init();
}


//======================================================================================================================
// setFont                                                                                                             =
//======================================================================================================================
void Painter::setFont(const char* fontFilename, uint nominalWidth, uint nominalHeight)
{
	font.reset(new Font(fontFilename, nominalWidth, nominalHeight));
}


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void Painter::init()
{
	// Map
	fontMap.loadRsrc("engine-rsrc/fontmap.png");
	columns = 16;
	rows = 8;
	tabSize = 4;

	// SProg
	sProg.loadRsrc("shaders/UiText.glsl");

	// Geom
	//float quadVertCoords[][2] = {{1.0, 0.0}, {0.0, 0.0}, {0.0, -1.0}, {1.0, -1.0}};
	float quadVertCoords[][2] = {{1.0, 1.0}, {0.0, 1.0}, {0.0, 0.0}, {1.0, 0.0}};
	qPositionsVbo.create(GL_ARRAY_BUFFER, sizeof(quadVertCoords), quadVertCoords, GL_STATIC_DRAW);

	ushort quadVertIndeces[2][3] = {{0, 1, 3}, {1, 2, 3}}; // 2 triangles
	qIndecesVbo.create(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadVertIndeces), quadVertIndeces, GL_STATIC_DRAW);

	qVao.create();
	qVao.attachArrayBufferVbo(qPositionsVbo, 0, 2, GL_FLOAT, false, 0, NULL);
	qVao.attachElementArrayBufferVbo(qIndecesVbo);
}


//======================================================================================================================
// drawText                                                                                                            =
//======================================================================================================================
void Painter::drawText(const char* text)
{
	// Set GL
	GlStateMachineSingleton::getInstance().enable(GL_BLEND);
	//GlStateMachineSingleton::getInstance().disable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GlStateMachineSingleton::getInstance().disable(GL_DEPTH_TEST);

	// SProg (some)
	sProg->bind();

	sProg->findUniVar("texture")->set(font->getMap(), 0);
	sProg->findUniVar("color")->set(&col);

	// Vao
	qVao.bind();

	// Iterate
	Vec2 p; // in NDC
	p.x() = 2.0 * pos.x() - 1.0;
	p.y() = 2.0 * -pos.y() - 1.0;
	const char* c = text;
	while(*c != '\0')
	{
		char cc = *c;
		// Check if special
		if(cc == ' ')
		{
			// do nothing
		}
		else if(cc == '\n')
		{
			/*p.y() += verticalMoving;
			p.x() = pos.x();
			++c;
			continue;*/
		}
		else if(cc == '\t') // tab
		{
			p.x() +=  font->getGlyphWidth(' ') * (tabSize - 1);
			cc = ' ';
		}
		else if(cc < ' ' || cc > '~') // out of range
		{
			cc = '~' + 1;
		}
		else
		{
			Mat3 trfM = Mat3::getIdentity();
			trfM(0, 0) = 2.0 * float(font->getGlyphWidth(cc)) / deviceWidth;
			trfM(1, 1) = 2.0 * float(font->getGlyphHeight(cc)) / deviceHeight;
			trfM(0, 2) = p.x() /*+ (font->getGlyphBearingX(cc) / float(deviceWidth))*/;

			trfM(1, 2) = 2 * (font->getGlyphBearingY(cc) - int(font->getGlyphHeight(cc))) / float(deviceHeight);

			sProg->findUniVar("transformation")->set(&trfM);
			sProg->findUniVar("textureTranformation")->set(&font->getGlyphTextureMatrix(cc));


			// Render
			glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_SHORT, 0);
		}

		// Inc
		++c;
		//p.x() += 2 * font->getGlyphAdvance(cc) / float(deviceWidth);
		p.x() += 2 * font->getGlyphWidth(cc) / float(deviceWidth);
	}
}


//======================================================================================================================
// drawFormatedText                                                                                                    =
//======================================================================================================================
void Painter::drawFormatedText(const char* format, ...)
{
	va_list ap;
	char text[512];

	va_start(ap, format);
	vsprintf(text, format, ap);
	va_end(ap);

	ASSERT(strlen(text) < 512);

	drawText(text);
}


} // end namespace
