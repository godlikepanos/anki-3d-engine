// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Pack.glsl"
#include "shaders/MsFsCommon.glsl"

//
// Input
//
#if NVIDIA_LINK_ERROR_WORKAROUND
layout(location = 0) in highp vec4 in_uv;
#else
layout(location = 0) in highp vec2 in_uv;
#endif

#if PASS == COLOR
layout(location = 1) in mediump vec3 in_normal;
layout(location = 2) in mediump vec4 in_tangent;
#if CALC_BITANGENT_IN_VERT
layout(location = 3) in mediump vec3 in_bitangent;
#endif
layout(location = 4) in mediump vec3 in_vertPosViewSpace;
layout(location = 5) in mediump vec3 in_eyeTangentSpace; // Parallax
layout(location = 6) in mediump vec3 in_normalTangentSpace; // Parallax
#endif

//
// Output
//
#if PASS == COLOR
layout(location = 0) out vec4 out_msRt0;
layout(location = 1) out vec4 out_msRt1;
layout(location = 2) out vec4 out_msRt2;
#define out_msRt0_DEFINED
#define out_msRt1_DEFINED
#define out_msRt2_DEFINED
#endif

// Getter
#if PASS == COLOR
#define getNormal_DEFINED
vec3 getNormal()
{
	return normalize(in_normal);
}
#endif

// Getter
#if PASS == COLOR
#define getTangent_DEFINED
vec4 getTangent()
{
	return in_tangent;
}
#endif

// Getter
#define getTextureCoord_DEFINED
vec2 getTextureCoord()
{
#if NVIDIA_LINK_ERROR_WORKAROUND
	return in_uv.xy;
#else
	return in_uv;
#endif
}

// Getter
#if PASS == COLOR
#define getPositionViewSpace_DEFINED
vec3 getPositionViewSpace()
{
#if TESSELLATION
	return vec3(0.0);
#else
	return in_vertPosViewSpace;
#endif
}
#endif

// Do normal mapping
#if PASS == COLOR
#define readNormalFromTexture_DEFINED
vec3 readNormalFromTexture(in vec3 normal, in vec4 tangent, in sampler2D map, in highp vec2 texCoords)
{
#if LOD > 0
	return normalize(normal);
#else
	// First read the texture
	vec3 nAtTangentspace = normalize((texture(map, texCoords).rgb - 0.5) * 2.0);

	vec3 n = normal; // Assume that getNormal() is called
	vec3 t = normalize(tangent.xyz);
#if CALC_BITANGENT_IN_VERT
	vec3 b = normalize(in_bitangent.xyz);
#else
	vec3 b = cross(n, t) * tangent.w;
#endif

	mat3 tbnMat = mat3(t, b, n);

	return tbnMat * nAtTangentspace;
#endif
}
#endif

// Do normal mapping by combining normal maps
#if PASS == COLOR
#define combineNormalFromTextures_DEFINED
vec3 combineNormalFromTextures(in vec3 normal,
	in vec4 tangent,
	in sampler2D map,
	in sampler2D map2,
	in highp vec2 texCoords,
	in float texCoords2Scale)
{
#if LOD > 0
	return normalize(normal);
#else
	// First read the textures
	vec3 nAtTangentspace0 = normalize((texture(map, texCoords).rgb - 0.5) * 2.0);
	vec3 nAtTangentspace1 = normalize((texture(map2, texCoords * texCoords2Scale).rgb - 0.5) * 2.0);

	vec3 nAtTangentspace = (nAtTangentspace0 + nAtTangentspace1) / 2.0;

	vec3 n = normal; // Assume that getNormal() is called
	vec3 t = normalize(tangent.xyz);
	vec3 b = cross(n, t) * tangent.w;

	mat3 tbnMat = mat3(t, b, n);

	return tbnMat * nAtTangentspace;
#endif
}
#endif

// Do environment mapping
#if PASS == COLOR
#define readEnvironmentColor_DEFINED
vec3 readEnvironmentColor(in vec3 vertPosViewSpace, in vec3 normal, in sampler2D map)
{
	// In case of normal mapping I could play with vertex's normal but this gives better results and its allready
	// computed

	vec3 u = normalize(vertPosViewSpace);
	vec3 r = reflect(u, normal);
	r.z += 1.0;
	float m = 2.0 * length(r);
	vec2 semTexCoords = r.xy / m + 0.5;

	vec3 semCol = texture(map, semTexCoords).rgb;
	return semCol;
}
#endif

// Using a 4-channel texture and a tolerance discard the fragment if the texture's alpha is less than the tolerance
#define readTextureRgbAlphaTesting_DEFINED
vec3 readTextureRgbAlphaTesting(in sampler2D map, in highp vec2 texCoords, in float tolerance)
{
#if PASS == COLOR
	vec4 col = vec4(texture(map, texCoords));
	if(col.a < tolerance)
	{
		discard;
	}
	return col.rgb;
#else // Depth
#if LOD > 0
	return vec3(0.0);
#else
	float a = float(texture(map, texCoords).a);
	if(a < tolerance)
	{
		discard;
	}
	return vec3(0.0);
#endif
#endif
}

// Just read the RGB color from texture
#if PASS == COLOR
#define readRgbFromTexture_DEFINED
vec3 readRgbFromTexture(in sampler2D tex, in highp vec2 texCoords)
{
	return vec3(texture(tex, texCoords)).rgb;
}
#endif

// Just read the RGB color from cube texture
#if PASS == COLOR
#define readRgbFromCubeTexture_DEFINED
vec3 readRgbFromCubeTexture(in samplerCube tex, in mediump vec3 texCoord)
{
	return texture(tex, texCoord).rgb;
}
#endif

// Just read the R color from texture
#if PASS == COLOR
#define readRFromTexture_DEFINED
float readRFromTexture(in sampler2D tex, in highp vec2 texCoords)
{
	return vec3(texture(tex, texCoords)).r;
}
#endif

#define computeTextureCoordParallax_DEFINED
vec2 computeTextureCoordParallax(in sampler2D heightMap, in vec2 uv, in float heightMapScale)
{
#if PASS == COLOR && LOD == 0
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
	float factor1 = in_vertPosViewSpace.z / -MAX_EFFECTIVE_DISTANCE;
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
#else
	return uv;
#endif
}

// Write the data to FAIs
#if PASS == COLOR
#define writeRts_DEFINED
void writeRts(in vec3 diffColor, // from 0 to 1
	in vec3 normal,
	in vec3 specularColor,
	in float roughness,
	in float subsurface,
	in float emission,
	in float metallic)
{
	GbufferInfo g;
	g.diffuse = diffColor;
	g.normal = normal;
	g.specular = specularColor;
	g.roughness = roughness;
	g.subsurface = subsurface;
	g.emission = emission;
	g.metallic = metallic;
	writeGBuffer(g, out_msRt0, out_msRt1, out_msRt2);
}
#endif
