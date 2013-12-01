// Common code for all fragment shaders of BS
#define DEFAULT_FLOAT_PRECISION mediump

#pragma anki include "shaders/Common.glsl"
#pragma anki include "shaders/MsBsCommon.glsl"
#pragma anki include "shaders/LinearDepth.glsl"

in vec2 vTexCoord;
flat in float vAlpha;

#if defined(PASS_COLOR)
layout(location = 0) out vec4 fColor;
#	define fColor_DEFINED
#endif

#if defined(PASS_COLOR)
#	define texture_DEFINED
#endif

#define getAlpha_DEFINED
float getAlpha()
{
	return vAlpha;
}

#define getPointCoord_DEFINED
#define getPointCoord() gl_PointCoord

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
	vec4 color = texture(tex, gl_PointCoord);
	color.a *= alpha;
	writeFais(color);
}
#endif

#if defined(PASS_COLOR)
#	define particleSoftTextureAlpha_DEFINED
void particleSoftTextureAlpha(in sampler2D depthMap, in sampler2D tex, 
	in float alpha)
{
	const vec2 screenSize = 
		vec2(1.0 / float(RENDERING_WIDTH), 1.0 / float(RENDERING_HEIGHT));
	float depth = texture(depthMap, gl_FragCoord.xy * screenSize).r;

	float delta = depth - gl_FragCoord.z;
	float softalpha = clamp(delta * 100.0, 0.0, 1.0);

	vec4 color = texture(tex, gl_PointCoord);
	color.a *= alpha * softalpha;
	writeFais(color);
}
#endif

#if defined(PASS_COLOR)
#	define particleSoftColorAlpha_DEFINED
void particleSoftColorAlpha(in sampler2D depthMap, in vec3 icolor, 
	in float alpha)
{
	const vec2 screenSize = 
		vec2(1.0 / float(RENDERING_WIDTH), 1.0 / float(RENDERING_HEIGHT));
	float depth = texture(depthMap, gl_FragCoord.xy * screenSize).r;

	float delta = depth - gl_FragCoord.z;
	float softalpha = clamp(delta * 100.0, 0.0, 1.0);

	vec2 pix = (1.0 - abs(gl_PointCoord * 2.0 - 1.0));
	float roundFactor = pix.x * pix.y;

	vec4 color;
	color.rgb = icolor;
	color.a = alpha * softalpha * roundFactor;
	writeFais(color);
}
#endif
