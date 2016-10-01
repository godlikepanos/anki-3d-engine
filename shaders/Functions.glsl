// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SHADERS_FUNCTIONS_GLSL
#define ANKI_SHADERS_FUNCTIONS_GLSL

vec3 dither(in vec3 col, in float C)
{
	vec3 vDither = vec3(dot(vec2(171.0, 231.0), gl_FragCoord.xy));
	vDither.rgb = fract(vDither.rgb / vec3(103.0, 71.0, 97.0));

	col = col * (255.0 / C) + vDither.rgb;
	col = floor(col) / 255.0;
	col *= C;

	return col;
}

float dither(in float col, in float C)
{
	float vDither = dot(vec2(171.0, 231.0), gl_FragCoord.xy);
	vDither = fract(vDither / 103.0);

	col = col * (255.0 / C) + vDither;
	col = floor(col) / 255.0;
	col *= C;

	return col;
}

// Convert to linear depth
float linearizeDepth(in float depth, in float zNear, in float zFar)
{
	return (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

// This is the optimal linearizeDepth where a=(f+n)/2n and b=(n-f)/2n
float linearizeDepthOptimal(in float depth, in float a, in float b)
{
	return 1.0 / (a + depth * b);
}

// Project a vector by knowing only the non zero values of a perspective matrix
vec4 projectPerspective(in vec4 vec, in float m00, in float m11, in float m22, in float m23)
{
	vec4 o;
	o.x = vec.x * m00;
	o.y = vec.y * m11;
	o.z = vec.z * m22 + vec.w * m23;
	o.w = -vec.z;
	return o;
}

// Stolen from shadertoy.com/view/4tyGDD
vec4 textureCatmullRom4Samples(sampler2D tex, vec2 uv, vec2 texSize)
{
	vec2 halff = 2.0 * fract(0.5 * uv * texSize - 0.25) - 1.0;
	vec2 f = fract(halff);
	vec2 sum0 = (2.0 * f - 3.5) * f + 0.5;
	vec2 sum1 = (2.0 * f - 2.5) * f - 0.5;
	vec4 w = vec4(f * sum0 + 1.0, f * sum1);
	vec4 pos = vec4((((-2.0 * f + 3.0) * f + 0.5) * f - 1.5) * f / (w.xy * texSize) + uv,
		(((-2.0 * f + 5.0) * f - 2.5) * f - 0.5) / (sum1 * texSize) + uv);
	w.xz *= halff.x * halff.y > 0.0 ? 1.0 : -1.0;

	return (texture(tex, pos.xy) * w.x + texture(tex, pos.zy) * w.z) * w.y
		+ (texture(tex, pos.xw) * w.x + texture(tex, pos.zw) * w.z) * w.w;
}

#endif
