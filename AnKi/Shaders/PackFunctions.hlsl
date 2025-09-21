// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Shaders/Common.hlsl>
#include <AnKi/Shaders/TonemappingFunctions.hlsl>

constexpr F32 kMinRoughness = 0.05;

/// Pack 3D normal to 2D vector
/// See the clean code in comments in revision < r467
template<typename T>
vector<T, 2> packNormal(vector<T, 3> normal)
{
	const T scale = 1.7777;
	const T scalar1 = (normal.z + T(1)) * (scale * T(2));
	return normal.xy / scalar1 + T(0.5);
}

/// Reverse the packNormal
template<typename T>
vector<T, 3> unpackNormal(const vector<T, 2> enc)
{
	const T scale = 1.7777;
	const vector<T, 2> nn = enc * (T(2) * scale) - scale;
	const T g = T(2) / (dot(nn.xy, nn.xy) + T(1));
	vector<T, 3> normal;
	normal.xy = g * nn.xy;
	normal.z = g - T(1);
	return normalize(normal);
}

// See http://johnwhite3d.blogspot.no/2017/10/signed-octahedron-normal-encoding.html
// Result in [0.0, 1.0]
template<typename T>
vector<T, 3> signedOctEncode(vector<T, 3> n)
{
	vector<T, 3> outn;

	const vector<T, 3> nabs = abs(n);
	n /= nabs.x + nabs.y + nabs.z;

	outn.y = n.y * T(0.5) + T(0.5);
	outn.x = n.x * T(0.5) + outn.y;
	outn.y = n.x * -T(0.5) + outn.y;

	outn.z = saturate(n.z * getMaxNumericLimit<T>());
	return outn;
}

// See http://johnwhite3d.blogspot.no/2017/10/signed-octahedron-normal-encoding.html
template<typename T>
vector<T, 3> signedOctDecode(vector<T, 3> n)
{
	vector<T, 3> outn;

	outn.x = n.x - n.y;
	outn.y = n.x + n.y - T(1);
	outn.z = n.z * T(2) - T(1);
	outn.z = outn.z * (1.0 - abs(outn.x) - abs(outn.y));

	outn = normalize(outn);
	return outn;
}

// Vectorized version. Assumes that v is in [0.0, 1.0]
template<typename T>
U32 packUnorm4x8(const vector<T, 4> value)
{
	const UVec4 packed = UVec4(value * T(255));
	return packed.x | (packed.y << 8u) | (packed.z << 16u) | (packed.w << 24u);
}

// Reverse of packUnorm4x8
template<typename T>
vector<T, 4> unpackUnorm4x8(const U32 value)
{
	const UVec4 packed = UVec4(value & 0xFF, (value >> 8u) & 0xFF, (value >> 16u) & 0xff, value >> 24u);
	return vector<T, 4>(packed) / T(255);
}

template<typename T>
U32 packSnorm4x8(vector<T, 4> value)
{
	const IVec4 packed = IVec4(round(clamp(value, T(-1), T(1)) * T(127))) & 0xFFu;
	return U32(packed.x | (packed.y << 8) | (packed.z << 16) | (packed.w << 24));
}

template<typename T>
vector<T, 4> unpackSnorm4x8(U32 value)
{
	const I32 signedValue = (I32)value;
	const IVec4 packed = IVec4(signedValue << 24, signedValue << 16, signedValue << 8, signedValue) >> 24;
	return clamp(vector<T, 4>(packed) / T(127), T(-1), T(1));
}

// Convert from RGB to YCbCr.
// The RGB should be in [0, 1] and the output YCbCr will be in [0, 1] as well.
template<typename T>
vector<T, 3> rgbToYCbCr(const vector<T, 3> rgb)
{
	const T y = dot(rgb, vector<T, 3>(0.299, 0.587, 0.114));
	const T cb = T(0.5) + dot(rgb, vector<T, 3>(-0.168736, -0.331264, 0.5));
	const T cr = T(0.5) + dot(rgb, vector<T, 3>(0.5, -0.418688, -0.081312));
	return vector<T, 3>(y, cb, cr);
}

// Convert the output of rgbToYCbCr back to RGB.
template<typename T>
vector<T, 3> yCbCrToRgb(const vector<T, 3> ycbcr)
{
	const T cb = ycbcr.y - T(0.5);
	const T cr = ycbcr.z - T(0.5);
	const T y = ycbcr.x;
	const T r = T(1.402) * cr;
	const T g = T(-0.344) * cb - T(0.714) * cr;
	const T b = T(1.772) * cb;
	return vector<T, 3>(r, g, b) + y;
}

// Pack a Vec2 to a single F32.
// comp should be in [0, 1] and the output will be in [0, 1].
template<typename T>
T packUnorm2ToUnorm1(const vector<T, 2> comp)
{
	return dot(round(comp * T(15)), Vec2(T(1) / T(255.0 / 16.0), T(1.0 / 255.0)));
}

// Unpack a single F32 to Vec2. Does the oposite of packUnorm2ToUnorm1.
template<typename T>
vector<T, 2> unpackUnorm1ToUnorm2(T c)
{
#if 1
	const T temp = c * T(255.0 / 16.0);
	const T a = floor(temp);
	const T b = temp - a; // b = fract(temp)
	return vector<T, 2>(a, b) * vector<T, 2>(1.0 / 15.0, 16.0 / 15.0);
#else
	const U32 temp = U32(c * 255.0);
	const U32 a = temp >> 4;
	const U32 b = temp & 0xF;
	return vector<T, 2>(a, b) / T(15);
#endif
}

// G-Buffer structure
template<typename T>
struct GbufferInfo
{
	vector<T, 3> m_diffuse;
	vector<T, 3> m_f0; ///< Freshnel at zero angles.
	vector<T, 3> m_normal;
	vector<T, 3> m_emission;
	T m_roughness;
	T m_metallic;
	T m_subsurface;
	Vec2 m_velocity;
};

struct GBufferPixelOut
{
	ANKI_RELAXED_PRECISION Vec4 m_rt0 : SV_Target0;
	ANKI_RELAXED_PRECISION Vec4 m_rt1 : SV_Target1;
	ANKI_RELAXED_PRECISION Vec4 m_rt2 : SV_Target2;
	Vec2 m_rt3 : SV_Target3;
};

// Populate the G buffer
template<typename T>
GBufferPixelOut packGBuffer(GbufferInfo<T> g)
{
	GBufferPixelOut output;

	const T packedSubsurfaceMetallic = packUnorm2ToUnorm1(vector<T, 2>(g.m_subsurface, g.m_metallic));

	const vector<T, 3> tonemappedEmission = reinhardTonemap(g.m_emission);

	output.m_rt0 = vector<T, 4>(g.m_diffuse, packedSubsurfaceMetallic);
	output.m_rt1 = vector<T, 4>(g.m_roughness, g.m_f0.x, tonemappedEmission.rb);

	const vector<T, 3> encNorm = signedOctEncode(g.m_normal);
	output.m_rt2 = vector<T, 4>(tonemappedEmission.g, encNorm);

	output.m_rt3 = g.m_velocity;

	return output;
}

template<typename T>
vector<T, 3> unpackDiffuseFromGBuffer(vector<T, 4> rt0, T metallic)
{
	return rt0.xyz *= T(1) - metallic;
}

template<typename T>
vector<T, 3> unpackNormalFromGBuffer(vector<T, 4> rt2)
{
	return signedOctDecode(rt2.yzw);
}

template<typename T>
T unpackRoughnessFromGBuffer(vector<T, 4> rt1, T minRoughness)
{
	T r = rt1.x;
	if(minRoughness > 0.0)
	{
		r = r * (T(1) - T(minRoughness)) + T(minRoughness);
	}
	return r;
}

template<typename T>
T unpackRoughnessFromGBuffer(vector<T, 4> rt1)
{
	return unpackRoughnessFromGBuffer<T>(rt1, kMinRoughness);
}

// Read part of the G-buffer
template<typename T>
void unpackGBufferNoVelocity(vector<T, 4> rt0, vector<T, 4> rt1, vector<T, 4> rt2, out GbufferInfo<T> g)
{
	g.m_diffuse = rt0.xyz;
	const vector<T, 2> unpackedSubsurfaceMetallic = unpackUnorm1ToUnorm2(rt0.w);
	g.m_subsurface = unpackedSubsurfaceMetallic.x;
	g.m_metallic = unpackedSubsurfaceMetallic.y;

	g.m_roughness = unpackRoughnessFromGBuffer(rt1);
	g.m_f0 = vector<T, 3>(rt1.y, rt1.y, rt1.y);
	g.m_emission = invertReinhardTonemap(vector<T, 3>(rt1.z, rt2.x, rt1.w));

	g.m_normal = signedOctDecode(rt2.yzw);

	g.m_velocity = getMaxNumericLimit<T>(); // Put something random

	// Compute reflectance
	g.m_f0 = lerp(g.m_f0, g.m_diffuse, g.m_metallic);

	// Compute diffuse
	g.m_diffuse *= T(1) - g.m_metallic;
}
