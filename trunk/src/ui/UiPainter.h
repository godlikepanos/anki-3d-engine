#ifndef UI_PAINTER_H
#define UI_PAINTER_H

#include "rsrc/RsrcPtr.h"
#include "m/Math.h"
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
		const Vec2& getPosition() const
		{
			return pos;
		}
		Vec2& getPosition()
		{
			return pos;
		}
		void setPosition(const Vec2& x)
		{
			pos = x;
		}

		const Vec4& getColor() const
		{
			return col;
		}
		Vec4& getColor()
		{
			return col;
		}
		void setColor(const Vec4& x)
		{
			col = x;
		}

		void setFont(const char* fontFilename, uint nominalWidth,
			uint nominalHeight);

		const UiFont& getFont() const
		{
			return *font;
		}
		/// @}

		void drawText(const char* text);
		void drawText(const std::string& str)
		{
			drawText(str.c_str());
		}
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
