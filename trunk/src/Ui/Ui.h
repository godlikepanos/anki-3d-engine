#ifndef UI_H
#define UI_H

#include "RsrcPtr.h"
#include "Math.h"
#include "Accessors.h"
#include "Vbo.h"
#include "Vao.h"


class Ui
{
	public:
		Ui();
		~Ui();

		/// @name Accessors
		/// @{
		GETTER_SETTER(Vec2, pos, getPosition, setPosition)
		GETTER_SETTER(Vec2, size, getSize, setSize)
		GETTER_SETTER(Vec4, col, getColor, setColor)
		/// @}

		void print(const char* text);

		void printf(const char* format, ...);

	private:
		RsrcPtr<Texture> fontMap;
		uint columns;
		uint rows;
		RsrcPtr<ShaderProg> sProg;

		Vec2 pos;
		Vec2 size;
		Vec4 col;

		Vbo qPositions;
		Vbo qIndeces;
		Vao qVao;
};


#endif
