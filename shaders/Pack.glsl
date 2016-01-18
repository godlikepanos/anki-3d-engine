// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SHADERS_PACK_GLSL
#define ANKI_SHADERS_PACK_GLSL

#include "shaders/Common.glsl"

/// Pack 3D normal to 2D vector
/// See the clean code in comments in revision < r467
vec2 packNormal(in vec3 normal)
{
    const float SCALE = 1.7777;
    float scalar1 = (normal.z + 1.0) * (SCALE * 2.0);
    return normal.xy / scalar1 + 0.5;
}

/// Reverse the packNormal
vec3 unpackNormal(in vec2 enc)
{
	const float SCALE = 1.7777;
	vec2 nn = enc * (2.0 * SCALE) - SCALE;
	float g = 2.0 / (dot(nn.xy, nn.xy) + 1.0);
	vec3 normal;
	normal.xy = g * nn.xy;
	normal.z = g - 1.0;
	return normalize(normal);
}

#if GL_ES || __VERSION__ < 400
// Vectorized version. See clean one at <= r1048
uint packUnorm4x8(in vec4 v)
{
	vec4 a = clamp(v, 0.0, 1.0) * 255.0;
	uvec4 b = uvec4(a) << uvec4(0U, 8U, 16U, 24U);
	uvec2 c = b.xy | b.zw;
	return c.x | c.y;
}

// Vectorized version. See clean one at <= r1048
vec4 unpackUnorm4x8(in highp uint u)
{
	uvec4 a = uvec4(u) >> uvec4(0U, 8U, 16U, 24U);
	uvec4 b = a & uvec4(0xFFU);
	vec4 c = vec4(b);

	return c * (1.0 / 255.0);
}
#endif

// Convert from RGB to YCbCr.
// The RGB should be in [0, 1] and the output YCbCr will be in [0, 1] as well.
vec3 rgbToYCbCr(in vec3 rgb)
{
    float y = dot(rgb, vec3(0.299, 0.587, 0.114));
	float cb = 0.5 + dot(rgb, vec3(-0.168736, -0.331264, 0.5));
	float cr = 0.5 + dot(rgb, vec3(0.5, -0.418688, -0.081312));
    return vec3(y, cb, cr);
}

// Convert the output of rgbToYCbCr back to RGB.
vec3 yCbCrToRgb(in vec3 ycbcr)
{
	float cb = ycbcr.y - 0.5;
	float cr = ycbcr.z - 0.5;
	float y = ycbcr.x;
	float r = 1.402 * cr;
	float g = -0.344 * cb - 0.714 * cr;
	float b = 1.772 * cb;
	return vec3(r, g, b) + y;
}

// Pack a vec2 to a single float.
// comp should be in [0, 1] and the output will be in [0, 1].
float packUnorm2ToUnorm1(in vec2 comp)
{
	return dot(round(comp * 15.0), vec2(1.0 / (255.0 / 16.0), 1.0 / 255.0));
}

// Unpack a single float to vec2. Does the oposite of packUnorm2ToUnorm1.
vec2 unpackUnorm1ToUnorm2(in float c)
{
#if 1
	float temp = c * (255.0 / 16.0);
	float a = floor(temp);
	float b = temp - a; // b = fract(temp)
	return vec2(a, b) * vec2(1.0 / 15.0, 16.0 / 15.0);
#else
	uint temp = uint(c * 255.0);
	uint a = temp >> 4;
	uint b = temp & 0xF;
	return vec2(a, b) / 15.0;
#endif
}

// Max emission. Keep as low as possible
const float MAX_EMISSION = 10.0;

// G-Buffer structure
struct GbufferInfo
{
	vec3 diffuse;
	vec3 specular;
	vec3 normal;
	float roughness;
	float metallic;
	float subsurface;
	float emission;
};

// Populate the G buffer
void writeGBuffer(in GbufferInfo g, 
	out vec4 rt0,
	out vec4 rt1,
	out vec4 rt2)
{
	rt0 = vec4(g.diffuse, g.subsurface);
	rt1 = vec4(g.specular, g.emission / MAX_EMISSION);
	rt2 = vec4(packNormal(g.normal), g.roughness, g.metallic);
}

// Read from G-buffer
void readNormalRoughnessMetallicFromGBuffer(
	in sampler2D rt2, in vec2 uv, inout GbufferInfo g)
{
	vec4 comp = texture(rt2, uv);
	g.normal = unpackNormal(comp.xy);
	g.roughness = comp.z;
	g.metallic = comp.w;
}

// Read from G-buffer
void readNormalFromGBuffer(in sampler2D rt2,  in vec2 uv, out vec3 normal)
{
	vec2 comp = texture(rt2, uv).rg;
	normal = unpackNormal(comp);
}

// Read from the G buffer
void readGBuffer(in sampler2D rt0,
	in sampler2D rt1,
	in sampler2D rt2,
	in vec2 uv,
	out GbufferInfo g)
{
	vec4 comp = texture(rt0, uv);
	g.diffuse = comp.xyz;
	g.subsurface = comp.w;

	comp = texture(rt1, uv);
	g.specular = comp.xyz;
	g.emission = comp.w * MAX_EMISSION;

	readNormalRoughnessMetallicFromGBuffer(rt2, uv, g);
}

#endif
