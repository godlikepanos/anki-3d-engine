#ifndef UI_PAINTER_H
#define UI_PAINTER_H

#include "RsrcPtr.h"
#include "Math.h"
#include "Accessors.h"
#include "Vbo.h"
#include "Vao.h"


namespace Ui {


class Painter
{
	public:
		Painter();

		/// @name Accessors
		/// @{
		GETTER_SETTER(Vec2, pos, getPosition, setPosition)
		GETTER_SETTER(Vec2, fontSize, getFontSize, setFontSize)
		GETTER_SETTER(Vec4, col, getColor, setColor)
		/// @}

		void drawText(const char* text);
		void drawFormatedText(const char* format, ...);

	private:
		/// @name Data
		/// @{
		RsrcPtr<Texture> fontMap;
		uint columns;
		uint rows;
		RsrcPtr<ShaderProg> sProg;

		Vec2 pos;
		Vec2 fontSize;
		Vec4 col;

		Vbo qPositionsVbo;
		Vbo qIndecesVbo;
		Vao qVao;
		/// @}

		void init();
};


}


#endif
