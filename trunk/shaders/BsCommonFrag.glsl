// Common code for all fragment shaders of BS
#define DEFAULT_FLOAT_PRECISION mediump

#pragma anki include "shaders/CommonFrag.glsl"
#pragma anki include "shaders/MsBsCommon.glsl"

#define vTexCoords_DEFINED
in highp vec2 vTexCoord;
#define vInstanceId_DEFINED
flat in highp uint vInstanceId;

#if defined(PASS_COLOR)
layout(location = 0) out vec4 fColor;
#	define fColor_DEFINED
#endif

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

#define particleAlpha_DEFINED
void particleAlpha(in sampler2D tex, in float alpha)
{
	vec4 color = texture(tex, vTexCoord);
	color.w *= alpha;
	writeFais(color);
}
