// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SHADERS_GBUFFER_COMMON_FRAG_GLSL
#define ANKI_SHADERS_GBUFFER_COMMON_FRAG_GLSL

#include <shaders/Pack.glsl>
#include <shaders/Common.glsl>

//
// Input
//
#if PASS == PASS_GB_FS
layout(location = 0) in highp Vec2 in_uv;
layout(location = 1) in mediump Vec3 in_normal;
layout(location = 2) in mediump Vec4 in_tangent;
#	if CALC_BITANGENT_IN_VERT
layout(location = 3) in mediump Vec3 in_bitangent;
#	endif
layout(location = 4) in mediump F32 in_distFromTheCamera; // Parallax
layout(location = 5) in mediump Vec3 in_eyeTangentSpace; // Parallax
layout(location = 6) in mediump Vec3 in_normalTangentSpace; // Parallax
#endif // PASS == PASS_GB_FS

//
// Output
//
#if PASS == PASS_GB_FS || PASS == PASS_EZ
layout(location = 0) out Vec4 out_msRt0;
layout(location = 1) out Vec4 out_msRt1;
layout(location = 2) out Vec4 out_msRt2;
#endif

//
// Functions
//
#if PASS == PASS_GB_FS
// Do normal mapping
Vec3 readNormalFromTexture(sampler2D map, highp Vec2 texCoords)
{
	// First read the texture
	Vec3 nAtTangentspace = normalize((texture(map, texCoords).rgb - 0.5) * 2.0);

	Vec3 n = normalize(in_normal);
	Vec3 t = normalize(in_tangent.xyz);
#	if CALC_BITANGENT_IN_VERT
	Vec3 b = normalize(in_bitangent.xyz);
#	else
	Vec3 b = cross(n, t) * in_tangent.w;
#	endif

	Mat3 tbnMat = Mat3(t, b, n);

	return tbnMat * nAtTangentspace;
}

// Using a 4-channel texture and a tolerance discard the fragment if the texture's alpha is less than the tolerance
Vec3 readTextureRgbAlphaTesting(sampler2D map, in highp Vec2 texCoords, F32 tolerance)
{
	Vec4 col = Vec4(texture(map, texCoords));
	if(col.a < tolerance)
	{
		discard;
	}

	return col.rgb;
}

Vec2 computeTextureCoordParallax(in sampler2D heightMap, in Vec2 uv, in F32 heightMapScale)
{
	const U32 MAX_SAMPLES = 25;
	const U32 MIN_SAMPLES = 1;
	const F32 MAX_EFFECTIVE_DISTANCE = 32.0;

	// Get that because we are sampling inside a loop
	Vec2 dPdx = dFdx(uv);
	Vec2 dPdy = dFdy(uv);

	Vec3 eyeTangentSpace = in_eyeTangentSpace;
	Vec3 normTangentSpace = in_normalTangentSpace;

	F32 parallaxLimit = -length(eyeTangentSpace.xy) / eyeTangentSpace.z;
	parallaxLimit *= heightMapScale;

	Vec2 offsetDir = normalize(eyeTangentSpace.xy);
	Vec2 maxOffset = offsetDir * parallaxLimit;

	Vec3 E = normalize(eyeTangentSpace);

	F32 factor0 = -dot(E, normTangentSpace);
	F32 factor1 = in_distFromTheCamera / -MAX_EFFECTIVE_DISTANCE;
	F32 factor = (1.0 - factor0) * (1.0 - factor1);
	F32 sampleCountf = mix(F32(MIN_SAMPLES), F32(MAX_SAMPLES), factor);

	F32 stepSize = 1.0 / sampleCountf;

	F32 crntRayHeight = 1.0;
	Vec2 crntOffset = Vec2(0.0);
	Vec2 lastOffset = Vec2(0.0);

	F32 lastSampledHeight = 1.0;
	F32 crntSampledHeight = 1.0;

	U32 crntSample = 0;

	U32 sampleCount = U32(sampleCountf);
	ANKI_LOOP while(crntSample < sampleCount)
	{
		crntSampledHeight = textureGrad(heightMap, uv + crntOffset, dPdx, dPdy).r;

		if(crntSampledHeight > crntRayHeight)
		{
			F32 delta1 = crntSampledHeight - crntRayHeight;
			F32 delta2 = (crntRayHeight + stepSize) - lastSampledHeight;
			F32 ratio = delta1 / (delta1 + delta2);

			crntOffset = mix(crntOffset, lastOffset, ratio);

			crntSample = sampleCount + 1;
		}
		else
		{
			crntSample++;

			crntRayHeight -= stepSize;

			lastOffset = crntOffset;
			crntOffset += stepSize * maxOffset;

			lastSampledHeight = crntSampledHeight;
		}
	}

	return uv + crntOffset;
}

// Write the data to FAIs
void writeRts(in Vec3 diffColor, // from 0 to 1
	in Vec3 normal,
	in Vec3 specularColor,
	in F32 roughness,
	in F32 subsurface,
	in Vec3 emission,
	in F32 metallic)
{
	GbufferInfo g;
	g.m_diffuse = diffColor;
	g.m_normal = normal;
	g.m_specular = specularColor;
	g.m_roughness = roughness;
	g.m_subsurface = subsurface;
	g.m_emission = (emission.r + emission.g + emission.b) / 3.0;
	g.m_metallic = metallic;
	writeGBuffer(g, out_msRt0, out_msRt1, out_msRt2);
}
#endif // PASS == PASS_GB_FS

#endif
