// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <shaders/Common.glsl>

/// Pack 3D normal to 2D vector
/// See the clean code in comments in revision < r467
Vec2 packNormal(in Vec3 normal)
{
	const F32 SCALE = 1.7777;
	F32 scalar1 = (normal.z + 1.0) * (SCALE * 2.0);
	return normal.xy / scalar1 + 0.5;
}

/// Reverse the packNormal
Vec3 unpackNormal(in Vec2 enc)
{
	const F32 SCALE = 1.7777;
	Vec2 nn = enc * (2.0 * SCALE) - SCALE;
	F32 g = 2.0 / (dot(nn.xy, nn.xy) + 1.0);
	Vec3 normal;
	normal.xy = g * nn.xy;
	normal.z = g - 1.0;
	return normalize(normal);
}

// See http://johnwhite3d.blogspot.no/2017/10/signed-octahedron-normal-encoding.html
// Result in [0.0, 1.0]
Vec3 signedOctEncode(Vec3 n)
{
	Vec3 outn;

	Vec3 nabs = abs(n);
	n /= nabs.x + nabs.y + nabs.z;

	outn.y = n.y * 0.5 + 0.5;
	outn.x = n.x * 0.5 + outn.y;
	outn.y = n.x * -0.5 + outn.y;

	outn.z = saturate(n.z * FLT_MAX);
	return outn;
}

// See http://johnwhite3d.blogspot.no/2017/10/signed-octahedron-normal-encoding.html
Vec3 signedOctDecode(Vec3 n)
{
	Vec3 outn;

	outn.x = n.x - n.y;
	outn.y = n.x + n.y - 1.0;
	outn.z = n.z * 2.0 - 1.0;
	outn.z = outn.z * (1.0 - abs(outn.x) - abs(outn.y));

	outn = normalize(outn);
	return outn;
}

// Vectorized version. See clean one at <= r1048
U32 newPackUnorm4x8(Vec4 v)
{
	Vec4 a = clamp(v, 0.0, 1.0) * 255.0;
	UVec4 b = UVec4(a) << UVec4(0U, 8U, 16U, 24U);
	UVec2 c = b.xy | b.zw;
	return c.x | c.y;
}

// Vectorized version. See clean one at <= r1048
Vec4 newUnpackUnorm4x8(highp U32 u)
{
	UVec4 a = UVec4(u) >> UVec4(0U, 8U, 16U, 24U);
	UVec4 b = a & UVec4(0xFFU);
	Vec4 c = Vec4(b);
	return c * (1.0 / 255.0);
}

// Convert from RGB to YCbCr.
// The RGB should be in [0, 1] and the output YCbCr will be in [0, 1] as well.
Vec3 rgbToYCbCr(in Vec3 rgb)
{
	F32 y = dot(rgb, Vec3(0.299, 0.587, 0.114));
	F32 cb = 0.5 + dot(rgb, Vec3(-0.168736, -0.331264, 0.5));
	F32 cr = 0.5 + dot(rgb, Vec3(0.5, -0.418688, -0.081312));
	return Vec3(y, cb, cr);
}

// Convert the output of rgbToYCbCr back to RGB.
Vec3 yCbCrToRgb(in Vec3 ycbcr)
{
	F32 cb = ycbcr.y - 0.5;
	F32 cr = ycbcr.z - 0.5;
	F32 y = ycbcr.x;
	F32 r = 1.402 * cr;
	F32 g = -0.344 * cb - 0.714 * cr;
	F32 b = 1.772 * cb;
	return Vec3(r, g, b) + y;
}

// Pack a Vec2 to a single F32.
// comp should be in [0, 1] and the output will be in [0, 1].
F32 packUnorm2ToUnorm1(in Vec2 comp)
{
	return dot(round(comp * 15.0), Vec2(1.0 / (255.0 / 16.0), 1.0 / 255.0));
}

// Unpack a single F32 to Vec2. Does the oposite of packUnorm2ToUnorm1.
Vec2 unpackUnorm1ToUnorm2(in F32 c)
{
#if 1
	F32 temp = c * (255.0 / 16.0);
	F32 a = floor(temp);
	F32 b = temp - a; // b = fract(temp)
	return Vec2(a, b) * Vec2(1.0 / 15.0, 16.0 / 15.0);
#else
	U32 temp = U32(c * 255.0);
	U32 a = temp >> 4;
	U32 b = temp & 0xF;
	return Vec2(a, b) / 15.0;
#endif
}

const F32 MAX_EMISSION = 20.0; // Max emission. Keep as low as possible
const F32 MIN_ROUGHNESS = 0.05;

// G-Buffer structure
struct GbufferInfo
{
	Vec3 m_diffuse;
	Vec3 m_specular;
	Vec3 m_normal;
	F32 m_roughness;
	F32 m_metallic;
	F32 m_subsurface;
	F32 m_emission;
	Vec2 m_velocity;
};

// Populate the G buffer
void writeGBuffer(GbufferInfo g, out Vec4 rt0, out Vec4 rt1, out Vec4 rt2, out Vec2 rt3)
{
	rt0 = Vec4(g.m_diffuse, g.m_subsurface);
	rt1 = Vec4(g.m_roughness, g.m_metallic, g.m_specular.x, 0.0);

	Vec3 encNorm = signedOctEncode(g.m_normal);
	rt2 = Vec4(encNorm.xy, g.m_emission / MAX_EMISSION, encNorm.z);

	rt3 = g.m_velocity;
}

// Read from G-buffer
Vec3 readNormalFromGBuffer(sampler2D rt2, Vec2 uv)
{
	return signedOctDecode(texture(rt2, uv).rga);
}

// Read the roughness from G-buffer
F32 readRoughnessFromGBuffer(sampler2D rt1, Vec2 uv)
{
	F32 r = textureLod(rt1, uv, 0.0).r;
	r = r * (1.0 - MIN_ROUGHNESS) + MIN_ROUGHNESS;
	return r;
}

// Read part of the G-buffer
void readGBuffer(sampler2D rt0, sampler2D rt1, sampler2D rt2, Vec2 uv, F32 lod, out GbufferInfo g)
{
	Vec4 comp = textureLod(rt0, uv, 0.0);
	g.m_diffuse = comp.xyz;
	g.m_subsurface = comp.w;

	comp = textureLod(rt1, uv, 0.0);
	g.m_roughness = comp.x;
	g.m_metallic = comp.y;
	g.m_specular = Vec3(comp.z);

	comp = textureLod(rt2, uv, 0.0);
	g.m_normal = signedOctDecode(comp.xyw);
	g.m_emission = comp.z * MAX_EMISSION;

	g.m_velocity = Vec2(FLT_MAX); // Put something random

	// Fix roughness
	g.m_roughness = g.m_roughness * (1.0 - MIN_ROUGHNESS) + MIN_ROUGHNESS;

	// Compute reflectance
	g.m_specular = mix(g.m_specular, g.m_diffuse, g.m_metallic);

	// Compute diffuse
	g.m_diffuse = g.m_diffuse - g.m_diffuse * g.m_metallic;
}
