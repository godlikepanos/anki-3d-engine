#include <stdio.h>
#include <stdarg.h>
#include "Ui.h"
#include "MainRenderer.h"
#include "App.h"
#include "Texture.h"
#include "Resource.h"

namespace Ui {


/*
=======================================================================================================================================
data members                                                                                                           =
=======================================================================================================================================
*/
static Texture* fontMap;

static ShaderProg* shader;

static float  initialX;
static float  fontW;
static float  fontH;
static float  color[4];
static bool   italic;
static float  crntX;
static float  crntY;



/*
=======================================================================================================================================
static funcs                                                                                                           =
=======================================================================================================================================
*/


// SetGL
static void SetGL()
{
	shader->bind();
	shader->findUniVar("fontMap")->setTexture(*fontMap, 0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	glColor4fv(color);

	// matrix stuff
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	app->getMainRenderer()->loadMatrix(Renderer::ortho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0));

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

}


// RestoreGL
static void RestoreGL()
{
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
}


// DrawChar
static void drawChar(char c)
{
	// first check for special chars
	if(c=='\n') // new line
	{
		//glTranslatef(initial_x, -font_h, 0.0);
		crntX = initialX;
		crntY -= fontH;
		return;
	}
	if(c=='\t') // tab
	{
		drawChar(' ');
		drawChar(' ');
		return;
	}
	if(c<' ' || c>'~') // out of range
	{
		c = '~'+1;
	}

	const int CHARS_PER_LINE = 16; // the chars that font_map.tga has per line
	const int LINES_NUM      = 8; // the lines of chars in font_map.tga

	// the uvs
	float charWidth = 1.0/float(CHARS_PER_LINE);
	float charHeight = 1.0/float(LINES_NUM);
	float uvTop = float(LINES_NUM - (c-' ')/CHARS_PER_LINE) / float(LINES_NUM);
	float uvLeft = float((c-' ')%CHARS_PER_LINE) / float(CHARS_PER_LINE);
	float uvRight = uvLeft + charWidth;
	float uvBottom = uvTop - charHeight;
	float uvs[4][2] = {{uvLeft, uvTop}, {uvLeft, uvBottom}, {uvRight, uvBottom}, {uvRight, uvTop}};

	// the coords
	float fwh = fontW/2.0 - crntX;
	float fhh = fontH/2.0 - crntY;
	float coords[4][2] = {{-fwh, fhh}, {-fwh, -fhh}, {fwh, -fhh}, {fwh, fhh}}; // from top left counterclockwise


	if(italic)
	{
		coords[0][0] += fontW/5.0;
		coords[3][0] += fontW/5.0;
	}

	glBegin(GL_QUADS);
		glTexCoord2fv(uvs[0]);  // top left
		glVertex2fv(coords[0]);
		glTexCoord2fv(uvs[1]);  // bottom left
		glVertex2fv(coords[1]);
		glTexCoord2fv(uvs[2]);  // botton right
		glVertex2fv(coords[2]);
		glTexCoord2fv(uvs[3]);  // top right
		glVertex2fv(coords[3]);
	glEnd();

	// draw outline
	/*if(1)
	glDisable(GL_TEXTURE_2D);
	glColor3f(0.0, 0.0, 1.0);
	glBegin(GL_LINES);
		glVertex2fv(coords[0]);
		glVertex2fv(coords[1]);
		glVertex2fv(coords[2]);
		glVertex2fv(coords[3]);
	glEnd();
	glEnable(GL_TEXTURE_2D);*/
	// end draw outline

	crntX += fontW*0.8;
	//glTranslatef(font_w*0.8f, 0.0, 0.0); // font_w*(float) to remove the space
}


/*
=======================================================================================================================================
non static funcs                                                                                                       =
=======================================================================================================================================
*/


// init
// exec after init SDL
void init()
{
	fontMap = Resource::textures.load("gfx/fontmapa.tga");
	fontMap->setTexParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//font_map->setTexParameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	shader = Resource::shaders.load("shaders/txt.glsl");
	setPos(0.0, 0.0);
	setFontWidth(0.05);
	setColor(Vec4(1.0, 1.0, 1.0, 1.0));
	italic = false;
}


// setFontWidth
// sets font width from the given param and the height fron the aspect ratio
void setFontWidth(float w_)
{
	// width
	fontW = w_;
	// height
	fontH = fontW * app->getMainRenderer()->getAspectRatio();
}


// setColor
void setColor(const Vec4& color_)
{
	for(int i=0; i<4; i++)
		color[i] = color_[i];
}

// setPos
void setPos(float x_, float y_)
{
	initialX = crntX = x_;
	crntY = y_;
}


// printf
void printf(const char* format, ...)
{
	va_list ap;
	char text[512];

	va_start(ap, format);		// Parses The String For Variables
		vsprintf(text, format, ap);  // And Converts Symbols To Actual Numbers
	va_end(ap);

	print(text);
}


//print
void print(const char* text)
{
	SetGL();
	//glTranslatef(crntX, crntY, 0.0);
	for(char* pc=const_cast<char*>(text); *pc!='\0'; pc++)
	{
		glLoadIdentity();
		//glTranslatef(crntX, crntY, 0.0);
		drawChar(*pc);
	}
	RestoreGL();
}


} // end namespace
