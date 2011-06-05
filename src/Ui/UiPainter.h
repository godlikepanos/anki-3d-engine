#ifndef UI_PAINTER_H
#define UI_PAINTER_H

#include <memory>
#include "RsrcPtr.h"
#include "Math.h"
#include "Accessors.h"
#include "Vbo.h"
#include "Vao.h"


namespace Ui {


class Font;


class Painter
{
	public:
		Painter(uint deviceWidth = 0, uint deviceHeight = 0);

		/// @name Accessors
		/// @{
		GETTER_SETTER(Vec2, pos, getPosition, setPosition)
		GETTER_SETTER(Vec2, fontSize, getFontSize, setFontSize)
		GETTER_SETTER(Vec4, col, getColor, setColor)
		//GETTER_SETTER(boost::array<uint, 2>, deviceSize, getDeviceSize, setDeviceSize)
		void setFont(const char* fontFilename, uint nominalWidth, uint nominalHeight);
		const Font& getFont() const {return *font;}
		/// @}

		void drawText(const char* text);
		void drawFormatedText(const char* format, ...);

	private:
		/// @name Data
		/// @{
		std::auto_ptr<Font> font;

		RsrcPtr<Texture> fontMap;
		uint columns;
		uint rows;
		RsrcPtr<ShaderProg> sProg;

		Vec2 pos;
		Vec2 fontSize;
		Vec4 col;
		uint tabSize;

		Vbo qPositionsVbo;
		Vbo qIndecesVbo;
		Vao qVao;

		uint deviceWidth; ///< The size of the device in pixels
		uint deviceHeight;
		/// @}

		void init();
};


}


#endif
