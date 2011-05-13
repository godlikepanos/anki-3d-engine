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
	fontMap.loadRsrc("engine-rsrc/fontmap.png");
	columns = 16;
	rows = 8;

	// SProg
	sProg.loadRsrc("shaders/Ui.glsl");

	// Geom
	float quadVertCoords[][2] = {{1.0, 0.0}, {0.0, 0.0}, {0.0, 1.0}, {1.0, 1.0}};
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
	GlStateMachineSingleton::getInstance().disable(GL_DEPTH);

	// SProg (some)
	sProg->bind();

	sProg->findUniVar("texture")->set(*fontMap, 0);
	sProg->findUniVar("color")->set(&col);

	// Vao
	qVao.bind();

	// Iterate
	Vec2 p = pos;
	const char* c = text;
	while(*c != '\0')
	{
		// Pos trf
		Mat3 trf = Mat3::getIdentity();
		trf(0, 0) = fontSize.x() * 2.0;
		trf(1, 1) = fontSize.y() * 2.0;
		trf(0, 2) = p.x() * 2.0 - 1.0;
		trf(1, 2) = p.y() * 2.0 - 1.0;

		sProg->findUniVar("transformation")->set(&trf);

		// Tex trf
		Mat3 texTrf = Mat3::getIdentity();
		sProg->findUniVar("textureTranformation")->set(&trf);

		// Render
		glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_SHORT, 0);

		// Inc
		++c;
		p.x() += fontSize.x();
	}
}


} // end namespace
