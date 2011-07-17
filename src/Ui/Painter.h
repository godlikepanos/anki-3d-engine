#ifndef UI_PAINTER_H
#define UI_PAINTER_H

#include "Resources/RsrcPtr.h"
#include "Math/Math.h"
#include "Util/Accessors.h"
#include "GfxApi/BufferObjects/Vbo.h"
#include "GfxApi/BufferObjects/Vao.h"
#include <boost/scoped_ptr.hpp>


namespace Ui {


class Font;


/// @todo
class Painter
{
	public:
		Painter(const Vec2& deviceSize);

		/// @name Accessors
		/// @{
		GETTER_SETTER(Vec2, pos, getPosition, setPosition)
		GETTER_SETTER(Vec4, col, getColor, setColor)
		void setFont(const char* fontFilename, uint nominalWidth,
			uint nominalHeight);
		const Font& getFont() const {return *font;}
		/// @}

		void drawText(const char* text);
		void drawText(const std::string& str) {drawText(str.c_str());}
		void drawFormatedText(const char* format, ...);

	private:
		/// @name Data
		/// @{
		boost::scoped_ptr<Font> font;
		RsrcPtr<ShaderProg> sProg;

		Vec2 pos;
		Vec4 col;
		uint tabSize;

		Vbo qPositionsVbo;
		Vbo qIndecesVbo;
		Vao qVao;

		Vec2 deviceSize; ///< The size of the device in pixels
		/// @}

		void init();
};


}


#endif
