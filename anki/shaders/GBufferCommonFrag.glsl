// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/shaders/Pack.glsl>
#include <anki/shaders/Common.glsl>

//
// Input
//
#if PASS == PASS_GB
layout(location = 0) in highp Vec2 in_uv;
layout(location = 1) in mediump Vec3 in_normal;
layout(location = 2) in mediump Vec4 in_tangent;
#	if CALC_BITANGENT_IN_VERT
layout(location = 3) in mediump Vec3 in_bitangent;
#	endif
layout(location = 4) in mediump F32 in_distFromTheCamera; // Parallax
layout(location = 5) in mediump Vec3 in_eyeTangentSpace; // Parallax
layout(location = 6) in mediump Vec3 in_normalTangentSpace; // Parallax

#	if VELOCITY
layout(location = 7) in mediump Vec2 in_velocity; // Velocity
#	endif
#endif // PASS == PASS_GB

//
// Output
//
#if PASS == PASS_GB || PASS == PASS_EZ
layout(location = 0) out Vec4 out_gbuffer0;
layout(location = 1) out Vec4 out_gbuffer1;
layout(location = 2) out Vec4 out_gbuffer2;
layout(location = 3) out Vec2 out_gbuffer3;
#endif

//
// Functions
//
#if PASS == PASS_GB
// Do normal mapping
Vec3 readNormalFromTexture(texture2D map, sampler sampl, highp Vec2 texCoords)
{
	// First read the texture
	const Vec3 nAtTangentspace = normalize((texture(map, sampl, texCoords).rgb - 0.5) * 2.0);

	const Vec3 n = normalize(in_normal);
	const Vec3 t = normalize(in_tangent.xyz);
#	if CALC_BITANGENT_IN_VERT
	const Vec3 b = normalize(in_bitangent.xyz);
#	else
	const Vec3 b = cross(n, t) * in_tangent.w;
#	endif

	const Mat3 tbnMat = Mat3(t, b, n);

	return tbnMat * nAtTangentspace;
}

// Using a 4-channel texture and a tolerance discard the fragment if the texture's alpha is less than the tolerance
Vec3 readTextureRgbAlphaTesting(texture2D map, sampler sampl, highp Vec2 texCoords, F32 tolerance)
{
	const Vec4 col = Vec4(texture(map, sampl, texCoords));
	if(col.a < tolerance)
	{
		discard;
	}

	return col.rgb;
}

Vec2 computeTextureCoordParallax(texture2D heightMap, sampler sampl, Vec2 uv, F32 heightMapScale)
{
	const U32 MAX_SAMPLES = 25;
	const U32 MIN_SAMPLES = 1;
	const F32 MAX_EFFECTIVE_DISTANCE = 32.0;

	// Get that because we are sampling inside a loop
	const Vec2 dPdx = dFdx(uv);
	const Vec2 dPdy = dFdy(uv);

	const Vec3 eyeTangentSpace = in_eyeTangentSpace;
	const Vec3 normTangentSpace = in_normalTangentSpace;

	F32 parallaxLimit = -length(eyeTangentSpace.xy) / eyeTangentSpace.z;
	parallaxLimit *= heightMapScale;

	const Vec2 offsetDir = normalize(eyeTangentSpace.xy);
	const Vec2 maxOffset = offsetDir * parallaxLimit;

	const Vec3 E = normalize(eyeTangentSpace);

	const F32 factor0 = -dot(E, normTangentSpace);
	const F32 factor1 = in_distFromTheCamera / -MAX_EFFECTIVE_DISTANCE;
	const F32 factor = saturate((1.0 - factor0) * (1.0 - factor1));
	const F32 sampleCountf = mix(F32(MIN_SAMPLES), F32(MAX_SAMPLES), factor);

	const F32 stepSize = 1.0 / sampleCountf;

	F32 crntRayHeight = 1.0;
	Vec2 crntOffset = Vec2(0.0);
	Vec2 lastOffset = Vec2(0.0);

	F32 lastSampledHeight = 1.0;
	F32 crntSampledHeight = 1.0;

	U32 crntSample = 0;

	const U32 sampleCount = U32(sampleCountf);
	ANKI_LOOP while(crntSample < sampleCount)
	{
		crntSampledHeight = textureGrad(heightMap, sampl, uv + crntOffset, dPdx, dPdy).r;

		if(crntSampledHeight > crntRayHeight)
		{
			const F32 delta1 = crntSampledHeight - crntRayHeight;
			const F32 delta2 = (crntRayHeight + stepSize) - lastSampledHeight;
			const F32 ratio = delta1 / (delta1 + delta2);

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
void writeRts(Vec3 diffColor, Vec3 normal, Vec3 specularColor, F32 roughness, F32 subsurface, Vec3 emission,
			  F32 metallic, Vec2 velocity)
{
	GbufferInfo g;
	g.m_diffuse = diffColor;
	g.m_normal = normal;
	g.m_specular = specularColor;
	g.m_roughness = roughness;
	g.m_subsurface = subsurface;
	g.m_emission = (emission.r + emission.g + emission.b) / 3.0;
	g.m_metallic = metallic;
	g.m_velocity = velocity;
	writeGBuffer(g, out_gbuffer0, out_gbuffer1, out_gbuffer2, out_gbuffer3);
}
#endif // PASS == PASS_GB
