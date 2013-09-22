// Common code for all fragment shaders of BS
#define DEFAULT_FLOAT_PRECISION mediump

#pragma anki include "shaders/CommonFrag.glsl"
#pragma anki include "shaders/MsBsCommon.glsl"
#pragma anki include "shaders/LinearDepth.glsl"

#define vTexCoord_DEFINED
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

#if defined(PASS_COLOR)
#	define particleAlpha_DEFINED
void particleAlpha(in sampler2D tex, in float alpha)
{
	vec4 color = texture(tex, vTexCoord);
	color.w *= alpha;
	writeFais(color);
}
#endif

#if defined(PASS_COLOR)
#	define softness_DEFINED
float softness(in sampler2D depthMap)
{
	float depth = texture(depthMap, gl_FragCoord.xy / vec2(960.0, 540.0)).r;

	float delta = depth - gl_FragCoord.z;
	
	if (delta > 0)
		return 1.0;
	else
		return 0.0;
}
#endif

