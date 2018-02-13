// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SHADERS_GBUFFER_COMMON_FRAG_GLSL
#define ANKI_SHADERS_GBUFFER_COMMON_FRAG_GLSL

#include "shaders/Pack.glsl"
#include "shaders/Common.glsl"

//
// Input
//
#if PASS == PASS_GB_FS
layout(location = 0) in highp vec2 in_uv;
layout(location = 1) in mediump vec3 in_normal;
layout(location = 2) in mediump vec4 in_tangent;
#	if CALC_BITANGENT_IN_VERT
layout(location = 3) in mediump vec3 in_bitangent;
#	endif
layout(location = 4) in mediump float in_distFromTheCamera; // Parallax
layout(location = 5) in mediump vec3 in_eyeTangentSpace; // Parallax
layout(location = 6) in mediump vec3 in_normalTangentSpace; // Parallax
#endif // PASS == PASS_GB_FS

//
// Output
//
#if PASS == PASS_GB_FS || PASS == PASS_EZ
layout(location = 0) out vec4 out_msRt0;
layout(location = 1) out vec4 out_msRt1;
layout(location = 2) out vec4 out_msRt2;
#endif

//
// Functions
//
#if PASS == PASS_GB_FS
// Do normal mapping
vec3 readNormalFromTexture(sampler2D map, highp vec2 texCoords)
{
	// First read the texture
	vec3 nAtTangentspace = normalize((texture(map, texCoords).rgb - 0.5) * 2.0);

	vec3 n = normalize(in_normal);
	vec3 t = normalize(in_tangent.xyz);
#	if CALC_BITANGENT_IN_VERT
	vec3 b = normalize(in_bitangent.xyz);
#	else
	vec3 b = cross(n, t) * in_tangent.w;
#	endif

	mat3 tbnMat = mat3(t, b, n);

	return tbnMat * nAtTangentspace;
}

// Using a 4-channel texture and a tolerance discard the fragment if the texture's alpha is less than the tolerance
vec3 readTextureRgbAlphaTesting(sampler2D map, in highp vec2 texCoords, float tolerance)
{
	vec4 col = vec4(texture(map, texCoords));
	if(col.a < tolerance)
	{
		discard;
	}

	return col.rgb;
}

vec2 computeTextureCoordParallax(in sampler2D heightMap, in vec2 uv, in float heightMapScale)
{
	const uint MAX_SAMPLES = 25;
	const uint MIN_SAMPLES = 1;
	const float MAX_EFFECTIVE_DISTANCE = 32.0;

	// Get that because we are sampling inside a loop
	vec2 dPdx = dFdx(uv);
	vec2 dPdy = dFdy(uv);

	vec3 eyeTangentSpace = in_eyeTangentSpace;
	vec3 normTangentSpace = in_normalTangentSpace;

	float parallaxLimit = -length(eyeTangentSpace.xy) / eyeTangentSpace.z;
	parallaxLimit *= heightMapScale;

	vec2 offsetDir = normalize(eyeTangentSpace.xy);
	vec2 maxOffset = offsetDir * parallaxLimit;

	vec3 E = normalize(eyeTangentSpace);

	float factor0 = -dot(E, normTangentSpace);
	float factor1 = in_distFromTheCamera / -MAX_EFFECTIVE_DISTANCE;
	float factor = (1.0 - factor0) * (1.0 - factor1);
	float sampleCountf = mix(float(MIN_SAMPLES), float(MAX_SAMPLES), factor);

	float stepSize = 1.0 / sampleCountf;

	float crntRayHeight = 1.0;
	vec2 crntOffset = vec2(0.0);
	vec2 lastOffset = vec2(0.0);

	float lastSampledHeight = 1.0;
	float crntSampledHeight = 1.0;

	uint crntSample = 0;

	uint sampleCount = uint(sampleCountf);
	while(crntSample < sampleCount)
	{
		crntSampledHeight = textureGrad(heightMap, uv + crntOffset, dPdx, dPdy).r;

		if(crntSampledHeight > crntRayHeight)
		{
			float delta1 = crntSampledHeight - crntRayHeight;
			float delta2 = (crntRayHeight + stepSize) - lastSampledHeight;
			float ratio = delta1 / (delta1 + delta2);

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
void writeRts(in vec3 diffColor, // from 0 to 1
	in vec3 normal,
	in vec3 specularColor,
	in float roughness,
	in float subsurface,
	in vec3 emission,
	in float metallic)
{
	GbufferInfo g;
	g.diffuse = diffColor;
	g.normal = normal;
	g.specular = specularColor;
	g.roughness = roughness;
	g.subsurface = subsurface;
	g.emission = (emission.r + emission.g + emission.b) / 3.0;
	g.metallic = metallic;
	writeGBuffer(g, out_msRt0, out_msRt1, out_msRt2);
}
#endif // PASS == PASS_GB_FS

#endif
