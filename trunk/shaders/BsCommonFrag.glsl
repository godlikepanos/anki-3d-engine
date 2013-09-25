// Common code for all fragment shaders of BS
#define DEFAULT_FLOAT_PRECISION mediump

#pragma anki include "shaders/Common.glsl"
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
#	define particleSoft_DEFINED
void particleSoft(in sampler2D depthMap, in sampler2D tex, in float alpha)
{
	const vec2 screenSize = 
		vec2(1.0 / float(RENDERING_WIDTH), 1.0 / float(RENDERING_HEIGHT));
	float depth = texture(depthMap, gl_FragCoord.xy * screenSize).r;

	float delta = depth - gl_FragCoord.z;
	float softalpha = clamp(delta * 100.0, 0.0, 1.0);

	vec4 color = texture(tex, vTexCoord);
	color.w *= alpha * softalpha;
	writeFais(color);
}
#endif

