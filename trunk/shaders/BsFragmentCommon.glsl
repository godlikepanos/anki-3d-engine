#define vTexCoords_DEFINED
in vec2 vTexCoords;

#if defined(PASS_COLOR)
layout(location = 0) out vec4 fColor;
#	define fColor_DEFINED
#endif

#pragma anki include "shaders/MaterialCommonFunctions.glsl"

#if defined(PASS_COLOR)
#	define texture_DEFINED
#endif

#if defined(PASS_COLOR)
#	define writeFais_DEFINED
void writeFais(in vec4 color)
{
	fColor = color;
}
#endif
