// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
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

// See http://johnwhite3d.blogspot.no/2017/10/signed-octahedron-normal-encoding.html
// Result in [0.0, 1.0]
vec3 signedOctEncode(vec3 n)
{
	vec3 outn;

	vec3 nabs = abs(n);
	n /= nabs.x + nabs.y + nabs.z;

	outn.y = n.y * 0.5 + 0.5;
	outn.x = n.x * 0.5 + outn.y;
	outn.y = n.x * -0.5 + outn.y;

	outn.z = saturate(n.z * FLT_MAX);
	return outn;
}

// See http://johnwhite3d.blogspot.no/2017/10/signed-octahedron-normal-encoding.html
vec3 signedOctDecode(vec3 n)
{
	vec3 outn;

	outn.x = n.x - n.y;
	outn.y = n.x + n.y - 1.0;
	outn.z = n.z * 2.0 - 1.0;
	outn.z = outn.z * (1.0 - abs(outn.x) - abs(outn.y));

	outn = normalize(outn);
	return outn;
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
const float MAX_EMISSION = 20.0;

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
void writeGBuffer(in GbufferInfo g, out vec4 rt0, out vec4 rt1, out vec4 rt2)
{
	rt0 = vec4(g.diffuse, g.subsurface);
	rt1 = vec4(g.roughness, g.metallic, g.specular.x, 0.0);

	vec3 encNorm = signedOctEncode(g.normal);
	rt2 = vec4(encNorm.xy, g.emission / MAX_EMISSION, encNorm.z);
}

// Read from G-buffer
void readNormalFromGBuffer(in sampler2D rt2, in vec2 uv, out vec3 normal)
{
	normal = signedOctDecode(texture(rt2, uv).rga);
}

// Read from the G buffer
void readGBuffer(in sampler2D rt0, in sampler2D rt1, in sampler2D rt2, in vec2 uv, in float lod, out GbufferInfo g)
{
	vec4 comp = textureLod(rt0, uv, 0.0);
	g.diffuse = comp.xyz;
	g.subsurface = comp.w;

	comp = textureLod(rt1, uv, 0.0);
	g.roughness = comp.x;
	g.metallic = comp.y;
	g.specular = vec3(comp.z);

	comp = textureLod(rt2, uv, 0.0);
	g.normal = signedOctDecode(comp.xyw);
	g.emission = comp.z * MAX_EMISSION;

	// Fix roughness
	const float MIN_ROUGHNESS = 0.05;
	g.roughness = g.roughness * (1.0 - MIN_ROUGHNESS) + MIN_ROUGHNESS;

	// Compute reflectance
	g.specular = mix(g.specular, g.diffuse, g.metallic);

	// Compute diffuse
	g.diffuse = g.diffuse - g.diffuse * g.metallic;
}

#endif
