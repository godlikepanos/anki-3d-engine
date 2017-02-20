// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SHADERS_FUNCTIONS_GLSL
#define ANKI_SHADERS_FUNCTIONS_GLSL

#include "shaders/Common.glsl"

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

float rand(vec2 n)
{
	return 0.5 + 0.5 * fract(sin(dot(n, vec2(12.9898, 78.233))) * 43758.5453);
}

vec3 nearestDepthUpscale(vec2 uv, sampler2D depthFull, sampler2D depthHalf, sampler2D colorTex, float depthThreshold)
{
	float fullDepth = texture(depthFull, uv).r;
	vec4 halfDepths = textureGather(depthHalf, uv, 0);
	vec4 diffs = abs(vec4(fullDepth) - halfDepths);
	vec3 color;

	if(all(lessThan(diffs, vec4(depthThreshold))))
	{
		// No major discontinuites, sample with bilinear
		color = texture(colorTex, uv).rgb;
	}
	else
	{
		// Some discontinuites, need to use the newUv
		vec4 r = textureGather(colorTex, uv, 0);
		vec4 g = textureGather(colorTex, uv, 1);
		vec4 b = textureGather(colorTex, uv, 2);

		float minDiff = diffs.x;
		uint comp = 0;

		if(diffs.y < minDiff)
		{
			comp = 1;
			minDiff = diffs.y;
		}

		if(diffs.z < minDiff)
		{
			comp = 2;
			minDiff = diffs.z;
		}

		if(diffs.w < minDiff)
		{
			comp = 3;
		}

		color = vec3(r[comp], g[comp], b[comp]);
	}

	return color;
}

float _calcDepthWeight(sampler2D depthLow, vec2 uv, float ref, vec2 linearDepthCf)
{
	float d = texture(depthLow, uv).r;
	float linearD = linearizeDepthOptimal(d, linearDepthCf.x, linearDepthCf.y);
	return 1.0 / (EPSILON + abs(ref - linearD));
}

vec4 _sampleAndWeight(sampler2D depthLow,
	sampler2D colorLow,
	vec2 lowInvSize,
	vec2 uv,
	vec2 offset,
	float ref,
	float weight,
	vec2 linearDepthCf,
	inout float normalize)
{
	uv += offset * lowInvSize;
	float dw = _calcDepthWeight(depthLow, uv, ref, linearDepthCf);
	vec4 v = texture(colorLow, uv);
	normalize += weight * dw;
	return v * dw * weight;
}

vec4 bilateralUpsample(
	sampler2D depthHigh, sampler2D depthLow, sampler2D colorLow, vec2 lowInvSize, vec2 uv, vec2 linearDepthCf)
{
	const vec3 WEIGHTS = vec3(0.25, 0.125, 0.0625);
	float depthRef = linearizeDepthOptimal(texture(depthHigh, uv).r, linearDepthCf.x, linearDepthCf.y);
	float normalize = 0.0;

	vec4 sum = _sampleAndWeight(
		depthLow, colorLow, lowInvSize, uv, vec2(0.0, 0.0), depthRef, WEIGHTS.x, linearDepthCf, normalize);
	sum += _sampleAndWeight(
		depthLow, colorLow, lowInvSize, uv, vec2(-1.0, 0.0), depthRef, WEIGHTS.y, linearDepthCf, normalize);
	sum += _sampleAndWeight(
		depthLow, colorLow, lowInvSize, uv, vec2(0.0, -1.0), depthRef, WEIGHTS.y, linearDepthCf, normalize);
	sum += _sampleAndWeight(
		depthLow, colorLow, lowInvSize, uv, vec2(1.0, 0.0), depthRef, WEIGHTS.y, linearDepthCf, normalize);
	sum += _sampleAndWeight(
		depthLow, colorLow, lowInvSize, uv, vec2(0.0, 1.0), depthRef, WEIGHTS.y, linearDepthCf, normalize);
	sum += _sampleAndWeight(
		depthLow, colorLow, lowInvSize, uv, vec2(1.0, 1.0), depthRef, WEIGHTS.z, linearDepthCf, normalize);
	sum += _sampleAndWeight(
		depthLow, colorLow, lowInvSize, uv, vec2(1.0, -1.0), depthRef, WEIGHTS.z, linearDepthCf, normalize);
	sum += _sampleAndWeight(
		depthLow, colorLow, lowInvSize, uv, vec2(-1.0, 1.0), depthRef, WEIGHTS.z, linearDepthCf, normalize);
	sum += _sampleAndWeight(
		depthLow, colorLow, lowInvSize, uv, vec2(-1.0, -1.0), depthRef, WEIGHTS.z, linearDepthCf, normalize);

	return sum / normalize;
}

vec3 getCubemapDirection(vec2 norm, uint faceIdx)
{
	vec3 zDir = vec3((faceIdx <= 1u) ? 1 : 0, (faceIdx & 2u) >> 1u, (faceIdx & 4u) >> 2u);
	zDir *= (((faceIdx & 1u) == 1u) ? -1.0 : 1.0);
	vec3 yDir = (faceIdx == 2u) ? vec3(0.0, 0.0, 1.0) : (faceIdx == 3u) ? vec3(0.0, 0.0, -1.0) : vec3(0.0, -1.0, 0.0);
	vec3 xDir = cross(zDir, yDir);
	return normalize(norm.x * xDir + norm.y * yDir + zDir);
}

#endif
