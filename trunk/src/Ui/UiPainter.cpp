#include <cstdarg>
#include "UiPainter.h"
#include "GlStateMachine.h"
#include "Texture.h"
#include "Logger.h"


namespace Ui {


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
Painter::Painter()
{
	init();
}


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
void Painter::init()
{
	// Map
	fontMap.loadRsrc("engine-rsrc/fontmap-big.png");
	columns = 16;
	rows = 8;
	tabSize = 4;

	// SProg
	sProg.loadRsrc("shaders/UiText.glsl");

	// Geom
	float quadVertCoords[][2] = {{1.0, 0.0}, {0.0, 0.0}, {0.0, -1.0}, {1.0, -1.0}};
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
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GlStateMachineSingleton::getInstance().disable(GL_DEPTH_TEST);

	// SProg (some)
	sProg->bind();

	sProg->findUniVar("texture")->set(*fontMap, 0);
	sProg->findUniVar("color")->set(&col);

	// Vao
	qVao.bind();

	// Iterate
	float horizontalMoving = fontSize.x() * 0.65;
	float verticalMoving = fontSize.y() /* * 0.7*/;
	Vec2 p = pos;
	const char* c = text;
	while(*c != '\0')
	{
		char cc = *c;
		// Check if special
		if(cc == '\n')
		{
			p.y() += verticalMoving;
			p.x() = pos.x();
			++c;
			continue;
		}
		if(cc == '\t') // tab
		{
			p.x() += horizontalMoving * (tabSize - 1);
			cc = ' ';
		}
		if(cc < ' ' || cc > '~') // out of range
		{
			cc = '~' + 1;
		}

		// Pos trf
		Mat3 trf = Mat3::getIdentity();
		trf(0, 0) = fontSize.x() * 2.0;
		trf(1, 1) = fontSize.y() * 2.0;
		trf(0, 2) = p.x() * 2.0 - 1.0;
		trf(1, 2) = -p.y() * 2.0 + 1.0;

		sProg->findUniVar("transformation")->set(&trf);

		// Tex trf
		uint i = cc - 32;
		uint crntRow = i / columns;
		uint crntColumn = i % columns;

		//INFO(crntRow << " " << crntColumn);

		Mat3 texTrf = Mat3::getIdentity();
		texTrf(0, 0) = 1.0 / columns;
		texTrf(1, 1) = 1.0 / rows;
		texTrf(0, 2) = crntColumn / float(columns);
		texTrf(1, 2) = 1.0 - crntRow / float(rows);

		sProg->findUniVar("textureTranformation")->set(&texTrf);

		// Render
		glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_SHORT, 0);

		// Inc
		++c;
		p.x() += horizontalMoving;
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
