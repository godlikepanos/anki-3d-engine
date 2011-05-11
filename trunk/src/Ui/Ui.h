#ifndef UI_H
#define UI_H


/*
namespace M {
	class Vec4;
}

namespace Ui { // begin namespace


extern void init(); // exec after init SDL
extern void setColor(const M::Vec4& color);
extern void setPos(float x_, float y_);
extern void setFontWidth(float w_);
extern void printf(const char* format, ...);
extern void print(const char* str);


} // end namespace */

#include "RsrcPtr.h"
#include "Math.h"


class Ui
{
	public:
		Ui();

		void setSize(float width, float height = -1.0);
		void setPos(const Vec2& pos);
		void setColor(const Vec4& col);
		void printf(const char* format, ...);

	private:
		RsrcPtr<Texture> fontMap;
		uint charsVertical;
		uint charsHorizontal;
		RsrcPtr<ShaderProg> sProg;
};


#endif
