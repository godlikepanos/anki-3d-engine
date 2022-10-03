// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Common.glsl>
#include <AnKi/Shaders/TonemappingFunctions.glsl>

const ANKI_RP F32 kMinRoughness = 0.05;

/// Pack 3D normal to 2D vector
/// See the clean code in comments in revision < r467
Vec2 packNormal(const Vec3 normal)
{
	const F32 scale = 1.7777;
	const F32 scalar1 = (normal.z + 1.0) * (scale * 2.0);
	return normal.xy / scalar1 + 0.5;
}

/// Reverse the packNormal
Vec3 unpackNormal(const Vec2 enc)
{
	const F32 scale = 1.7777;
	const Vec2 nn = enc * (2.0 * scale) - scale;
	const F32 g = 2.0 / (dot(nn.xy, nn.xy) + 1.0);
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

	const Vec3 nabs = abs(n);
	n /= nabs.x + nabs.y + nabs.z;

	outn.y = n.y * 0.5 + 0.5;
	outn.x = n.x * 0.5 + outn.y;
	outn.y = n.x * -0.5 + outn.y;

	outn.z = saturate(n.z * kMaxF32);
	return outn;
}

// See http://johnwhite3d.blogspot.no/2017/10/signed-octahedron-normal-encoding.html
Vec3 signedOctDecode(const Vec3 n)
{
	Vec3 outn;

	outn.x = n.x - n.y;
	outn.y = n.x + n.y - 1.0;
	outn.z = n.z * 2.0 - 1.0;
	outn.z = outn.z * (1.0 - abs(outn.x) - abs(outn.y));

	outn = normalize(outn);
	return outn;
}

// Vectorized version. Assumes that v is in [0.0, 1.0]
U32 newPackUnorm4x8(const Vec4 v)
{
	Vec4 a = v * 255.0;
	UVec4 b = UVec4(a) << UVec4(0u, 8u, 16u, 24u);
	UVec2 c = b.xy | b.zw;
	return c.x | c.y;
}

// Vectorized version
Vec4 newUnpackUnorm4x8(const U32 u)
{
	const UVec4 a = UVec4(u) >> UVec4(0u, 8u, 16u, 24u);
	const UVec4 b = a & UVec4(0xFFu);
	const Vec4 c = Vec4(b);
	return c * (1.0 / 255.0);
}

// Convert from RGB to YCbCr.
// The RGB should be in [0, 1] and the output YCbCr will be in [0, 1] as well.
Vec3 rgbToYCbCr(const Vec3 rgb)
{
	const F32 y = dot(rgb, Vec3(0.299, 0.587, 0.114));
	const F32 cb = 0.5 + dot(rgb, Vec3(-0.168736, -0.331264, 0.5));
	const F32 cr = 0.5 + dot(rgb, Vec3(0.5, -0.418688, -0.081312));
	return Vec3(y, cb, cr);
}

// Convert the output of rgbToYCbCr back to RGB.
Vec3 yCbCrToRgb(const Vec3 ycbcr)
{
	const F32 cb = ycbcr.y - 0.5;
	const F32 cr = ycbcr.z - 0.5;
	const F32 y = ycbcr.x;
	const F32 r = 1.402 * cr;
	const F32 g = -0.344 * cb - 0.714 * cr;
	const F32 b = 1.772 * cb;
	return Vec3(r, g, b) + y;
}

// Pack a Vec2 to a single F32.
// comp should be in [0, 1] and the output will be in [0, 1].
F32 packUnorm2ToUnorm1(const Vec2 comp)
{
	return dot(round(comp * 15.0), Vec2(1.0 / (255.0 / 16.0), 1.0 / 255.0));
}

// Unpack a single F32 to Vec2. Does the oposite of packUnorm2ToUnorm1.
Vec2 unpackUnorm1ToUnorm2(F32 c)
{
#if 1
	const F32 temp = c * (255.0 / 16.0);
	const F32 a = floor(temp);
	const F32 b = temp - a; // b = fract(temp)
	return Vec2(a, b) * Vec2(1.0 / 15.0, 16.0 / 15.0);
#else
	const U32 temp = U32(c * 255.0);
	const U32 a = temp >> 4;
	const U32 b = temp & 0xF;
	return Vec2(a, b) / 15.0;
#endif
}

// G-Buffer structure
struct GbufferInfo
{
	ANKI_RP Vec3 m_diffuse;
	ANKI_RP Vec3 m_f0; ///< Freshnel at zero angles.
	ANKI_RP Vec3 m_normal;
	ANKI_RP F32 m_roughness;
	ANKI_RP F32 m_metallic;
	ANKI_RP F32 m_subsurface;
	ANKI_RP Vec3 m_emission;
	Vec2 m_velocity;
};

// Populate the G buffer
void packGBuffer(GbufferInfo g, out Vec4 rt0, out Vec4 rt1, out Vec4 rt2, out Vec2 rt3)
{
	const F32 packedSubsurfaceMetallic = packUnorm2ToUnorm1(Vec2(g.m_subsurface, g.m_metallic));

	const Vec3 tonemappedEmission = reinhardTonemap(g.m_emission);

	rt0 = Vec4(g.m_diffuse, packedSubsurfaceMetallic);
	rt1 = Vec4(g.m_roughness, g.m_f0.x, tonemappedEmission.rb);

	const Vec3 encNorm = signedOctEncode(g.m_normal);
	rt2 = Vec4(tonemappedEmission.g, encNorm);

	rt3 = g.m_velocity;
}

ANKI_RP Vec3 unpackDiffuseFromGBuffer(ANKI_RP Vec4 rt0, ANKI_RP F32 metallic)
{
	return rt0.xyz *= 1.0 - metallic;
}

ANKI_RP Vec3 unpackNormalFromGBuffer(ANKI_RP Vec4 rt2)
{
	return signedOctDecode(rt2.yzw);
}

ANKI_RP F32 unpackRoughnessFromGBuffer(ANKI_RP Vec4 rt1)
{
	ANKI_RP F32 r = rt1.x;
	r = r * (1.0 - kMinRoughness) + kMinRoughness;
	return r;
}

// Read part of the G-buffer
void unpackGBufferNoVelocity(ANKI_RP Vec4 rt0, ANKI_RP Vec4 rt1, ANKI_RP Vec4 rt2, out GbufferInfo g)
{
	g.m_diffuse = rt0.xyz;
	const Vec2 unpackedSubsurfaceMetallic = unpackUnorm1ToUnorm2(rt0.w);
	g.m_subsurface = unpackedSubsurfaceMetallic.x;
	g.m_metallic = unpackedSubsurfaceMetallic.y;

	g.m_roughness = unpackRoughnessFromGBuffer(rt1);
	g.m_f0 = Vec3(rt1.y);
	g.m_emission = invertReinhardTonemap(Vec3(rt1.z, rt2.x, rt1.w));

	g.m_normal = signedOctDecode(rt2.yzw);

	g.m_velocity = Vec2(kMaxF32); // Put something random

	// Compute reflectance
	g.m_f0 = mix(g.m_f0, g.m_diffuse, g.m_metallic);

	// Compute diffuse
	g.m_diffuse *= 1.0 - g.m_metallic;
}
