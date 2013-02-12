#ifndef ANKI_UI_UI_PAINTER_H
#define ANKI_UI_UI_PAINTER_H

#include "anki/ui/UiCommon.h"
#include "anki/gl/Gl.h"

namespace anki {

// Forward
class UiFont;

/// Gradient type
struct UiGrandient
{
	enum GradientType
	{
		GT_ANGULAR,
		GT_RADIAL
	};

	GradientType type = GT_ANGULAR;
	Array<UiColor, 3> colors = {{UiColor(1.0), UiColor(0.5), UiColor(0.0)}};
	U8 colorsCount = 2;
	F32 angle = 0.0; ///< In rad. Default is horizontal
};

/// UI painer
class UiPainter
{
public:
	/// Method for coloring
	enum ColoringMethod
	{
		CM_CONSTANT_COLOR,
		CM_GRADIENT
	};

	UiPainter(const Vec2& deviceSize);

	/// @name Accessors
	/// @{
	const UiPosition& getPosition() const
	{
		return pos;
	}
	void setPosition(const UiPosition& x)
	{
		pos = x;
	}

	const UiColor& getColor() const
	{
		return col;
	}
	void setColor(const UiColor& x)
	{
		col = x;
	}
	/// @}

	/// @name Fill methods
	/// @{
	void fillRect(const UiRect& rect);
	void fillEllipse(const UiRect& rect);
	void fillRoundedRect(const UiRect& rect, F32 xRadius, F32 yRadius);
	/// @}

	/// @name Draw methods
	/// @{
	void drawText(const char* text);
	void drawFormatedText(const char* format, ...);

	void drawLines(const UiPosition* positions, U32 positionsCount);
	void drawRect(const UiRect& rect);
	void drawEllipse(const UiRect& rect);
	void drawRoundedRect(const UiRect& rect, F32 xRadius, F32 yRadius);
	/// @}

private:
	enum Shader
	{
		S_FILL,
		S_GRADIENT_ANGULAR_COLORS_2,
		S_GRADIENT_ANGULAR_COLORS_3,
		S_GRADIENT_RADIAL_COLORS_2,
		S_GRADIENT_RADIAL_COLORS_3,
		S_COUNT
	};

	/// @name Data
	/// @{
	UiFont* font = nullptr;
	Array<ShaderProgram, S_COUNT> progs;

	Vbo qPositionsVbo;
	Vbo qIndecesVbo;
	Vao qVao;

	Vec2 deviceSize; ///< The size of the device in pixels
	/// @}

	/// @name State machine data
	/// @{
	UiPosition pos;
	UiColor col;
	Gradient grad;
	ColoringMethod fillMethod;
	ColoringMethod linesMethod;
	/// @}

	/// @name Oprtimizations
	/// @{
	/// @}

	void init();
};


} // end namespace


#endif
