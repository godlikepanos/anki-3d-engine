#ifndef UI_PAINTER_H
#define UI_PAINTER_H

#include "rsrc/RsrcPtr.h"
#include "m/Math.h"
#include "util/Accessors.h"
#include "gl/Vbo.h"
#include "gl/Vao.h"
#include <boost/scoped_ptr.hpp>


class UiFont;


/// @todo
class UiPainter
{
	public:
		UiPainter(const Vec2& deviceSize);

		/// @name Accessors
		/// @{
		GETTER_SETTER(Vec2, pos, getPosition, setPosition)
		GETTER_SETTER(Vec4, col, getColor, setColor)
		void setFont(const char* fontFilename, uint nominalWidth,
			uint nominalHeight);
		const UiFont& getFont() const {return *font;}
		/// @}

		void drawText(const char* text);
		void drawText(const std::string& str) {drawText(str.c_str());}
		void drawFormatedText(const char* format, ...);

	private:
		/// @name Data
		/// @{
		boost::scoped_ptr<UiFont> font;
		RsrcPtr<ShaderProgram> sProg;

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


#endif
