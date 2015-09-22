// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma anki include "shaders/Common.glsl"

#if !GL_ES && __VERSION__ > 400
layout(early_fragment_tests) in;
#endif

#pragma anki include "shaders/Pack.glsl"
#pragma anki include "shaders/MsFsCommon.glsl"

//==============================================================================
// Variables                                                                   =
//==============================================================================

//
// Input
//
layout(location = 0) in mediump vec2 in_uv;
#if PASS == COLOR
layout(location = 1) in mediump vec3 in_normal;
layout(location = 2) in mediump vec4 in_tangent;
layout(location = 3) in mediump vec3 in_vertPosViewSpace;
#endif

//
// Output
//
#if PASS == COLOR
layout(location = 0) out vec4 out_msRt0;
layout(location = 1) out vec4 out_msRt1;
layout(location = 2) out vec4 out_msRt2;
#	define out_msRt0_DEFINED
#	define out_msRt1_DEFINED
#	define out_msRt2_DEFINED
#endif

//==============================================================================
// Functions                                                                   =
//==============================================================================

// Getter
#if PASS == COLOR
#	define getNormal_DEFINED
vec3 getNormal()
{
	return normalize(in_normal);
}
#endif

// Getter
#if PASS == COLOR
#	define getTangent_DEFINED
vec4 getTangent()
{
	return in_tangent;
}
#endif

// Getter
#define getTextureCoord_DEFINED
vec2 getTextureCoord()
{
	return in_uv;
}

// Getter
#if PASS == COLOR
#	define getPositionViewSpace_DEFINED
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
#	define readNormalFromTexture_DEFINED
vec3 readNormalFromTexture(in vec3 normal, in vec4 tangent,
	in sampler2D map, in highp vec2 texCoords)
{
#	if LOD > 0
	return normalize(normal);
#	else
	// First read the texture
	vec3 nAtTangentspace = normalize((texture(map, texCoords).rgb - 0.5) * 2.0);

	vec3 n = normal; // Assume that getNormal() is called
	vec3 t = normalize(tangent.xyz);
	vec3 b = cross(n, t) * tangent.w;

	mat3 tbnMat = mat3(t, b, n);

	return tbnMat * nAtTangentspace;
#	endif
}
#endif

// Do normal mapping by combining normal maps
#if PASS == COLOR
#	define combineNormalFromTextures_DEFINED
vec3 combineNormalFromTextures(in vec3 normal, in vec4 tangent,
	in sampler2D map, in sampler2D map2, in highp vec2 texCoords,
	in float texCoords2Scale)
{
#	if LOD > 0
	return normalize(normal);
#	else
	// First read the textures
	vec3 nAtTangentspace0 =
		normalize((texture(map, texCoords).rgb - 0.5) * 2.0);
	vec3 nAtTangentspace1 =
		normalize((texture(map2, texCoords * texCoords2Scale).rgb - 0.5) * 2.0);

	vec3 nAtTangentspace = (nAtTangentspace0 + nAtTangentspace1) / 2.0;

	vec3 n = normal; // Assume that getNormal() is called
	vec3 t = normalize(tangent.xyz);
	vec3 b = cross(n, t) * tangent.w;

	mat3 tbnMat = mat3(t, b, n);

	return tbnMat * nAtTangentspace;
#	endif
}
#endif

// Do environment mapping
#if PASS == COLOR
#	define readEnvironmentColor_DEFINED
vec3 readEnvironmentColor(in vec3 vertPosViewSpace, in vec3 normal,
	in sampler2D map)
{
	// In case of normal mapping I could play with vertex's normal but this
	// gives better results and its allready computed

	vec3 u = normalize(vertPosViewSpace);
	vec3 r = reflect(u, normal);
	r.z += 1.0;
	float m = 2.0 * length(r);
	vec2 semTexCoords = r.xy / m + 0.5;

	vec3 semCol = texture(map, semTexCoords).rgb;
	return semCol;
}
#endif

// Using a 4-channel texture and a tolerance discard the fragment if the
// texture's alpha is less than the tolerance
#define readTextureRgbAlphaTesting_DEFINED
vec3 readTextureRgbAlphaTesting(
	in sampler2D map,
	in highp vec2 texCoords,
	in float tolerance)
{
#if PASS == COLOR
	vec4 col = vec4(texture(map, texCoords));
	if(col.a < tolerance)
	{
		discard;
	}
	return col.rgb;
#else // Depth
#	if LOD > 0
	return vec3(0.0);
#	else
	float a = float(texture(map, texCoords).a);
	if(a < tolerance)
	{
		discard;
	}
	return vec3(0.0);
#	endif
#endif
}

// Just read the RGB color from texture
#if PASS == COLOR
#	define readRgbFromTexture_DEFINED
vec3 readRgbFromTexture(in sampler2D tex, in highp vec2 texCoords)
{
	return vec3(texture(tex, texCoords)).rgb;
}
#endif

// Just read the RGB color from cube texture
#if PASS == COLOR
#	define readRgbFromCubeTexture_DEFINED
vec3 readRgbFromCubeTexture(in samplerCube tex, in mediump vec3 texCoord)
{
	return texture(tex, texCoord).rgb;
}
#endif

// Just read the R color from texture
#if PASS == COLOR
#	define readRFromTexture_DEFINED
float readRFromTexture(in sampler2D tex, in highp vec2 texCoords)
{
	return vec3(texture(tex, texCoords)).r;
}
#endif

// Write the data to FAIs
#if PASS == COLOR
#	define writeRts_DEFINED
void writeRts(
	in vec3 diffColor, // from 0 to 1
	in vec3 normal,
	in vec3 specularColor,
	in float roughness,
	in float subsurface,
	in float emission)
{
	writeGBuffer(diffColor, normal, specularColor, roughness, subsurface,
		emission, out_msRt0, out_msRt1, out_msRt2);
}
#endif
