// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// WARNING: THIS FILE IS AUTO-GENERATED. DON'T CHANGE IT BY HAND.
// This file contains some convenience texture functions.

#pragma once

#if defined(ANKI_FRAGMENT_SHADER)
vec4 texture(texture1D tex, sampler sampl, float P, float bias)
{
	return texture(sampler1D(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
uvec4 texture(utexture1D tex, sampler sampl, float P, float bias)
{
	return texture(usampler1D(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
ivec4 texture(itexture1D tex, sampler sampl, float P, float bias)
{
	return texture(isampler1D(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
vec4 texture(texture1D tex, sampler sampl, float P)
{
	return texture(sampler1D(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
uvec4 texture(utexture1D tex, sampler sampl, float P)
{
	return texture(usampler1D(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
ivec4 texture(itexture1D tex, sampler sampl, float P)
{
	return texture(isampler1D(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
vec4 texture(texture2D tex, sampler sampl, vec2 P, float bias)
{
	return texture(sampler2D(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
uvec4 texture(utexture2D tex, sampler sampl, vec2 P, float bias)
{
	return texture(usampler2D(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
ivec4 texture(itexture2D tex, sampler sampl, vec2 P, float bias)
{
	return texture(isampler2D(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
vec4 texture(texture2D tex, sampler sampl, vec2 P)
{
	return texture(sampler2D(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
uvec4 texture(utexture2D tex, sampler sampl, vec2 P)
{
	return texture(usampler2D(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
ivec4 texture(itexture2D tex, sampler sampl, vec2 P)
{
	return texture(isampler2D(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
vec4 texture(texture3D tex, sampler sampl, vec3 P, float bias)
{
	return texture(sampler3D(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
uvec4 texture(utexture3D tex, sampler sampl, vec3 P, float bias)
{
	return texture(usampler3D(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
ivec4 texture(itexture3D tex, sampler sampl, vec3 P, float bias)
{
	return texture(isampler3D(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
vec4 texture(texture3D tex, sampler sampl, vec3 P)
{
	return texture(sampler3D(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
uvec4 texture(utexture3D tex, sampler sampl, vec3 P)
{
	return texture(usampler3D(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
ivec4 texture(itexture3D tex, sampler sampl, vec3 P)
{
	return texture(isampler3D(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
vec4 texture(textureCube tex, sampler sampl, vec3 P, float bias)
{
	return texture(samplerCube(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
uvec4 texture(utextureCube tex, sampler sampl, vec3 P, float bias)
{
	return texture(usamplerCube(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
ivec4 texture(itextureCube tex, sampler sampl, vec3 P, float bias)
{
	return texture(isamplerCube(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
vec4 texture(textureCube tex, sampler sampl, vec3 P)
{
	return texture(samplerCube(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
uvec4 texture(utextureCube tex, sampler sampl, vec3 P)
{
	return texture(usamplerCube(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
ivec4 texture(itextureCube tex, sampler sampl, vec3 P)
{
	return texture(isamplerCube(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
float texture(texture1D tex, samplerShadow sampl, vec3 P, float bias)
{
	return texture(sampler1DShadow(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
float texture(texture1D tex, samplerShadow sampl, vec3 P)
{
	return texture(sampler1DShadow(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
float texture(texture2D tex, samplerShadow sampl, vec3 P, float bias)
{
	return texture(sampler2DShadow(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
float texture(texture2D tex, samplerShadow sampl, vec3 P)
{
	return texture(sampler2DShadow(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
float texture(textureCube tex, samplerShadow sampl, vec4 P, float bias)
{
	return texture(samplerCubeShadow(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
float texture(textureCube tex, samplerShadow sampl, vec4 P)
{
	return texture(samplerCubeShadow(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
vec4 texture(texture2DArray tex, sampler sampl, vec3 P, float bias)
{
	return texture(sampler2DArray(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
uvec4 texture(utexture2DArray tex, sampler sampl, vec3 P, float bias)
{
	return texture(usampler2DArray(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
ivec4 texture(itexture2DArray tex, sampler sampl, vec3 P, float bias)
{
	return texture(isampler2DArray(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
vec4 texture(texture2DArray tex, sampler sampl, vec3 P)
{
	return texture(sampler2DArray(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
uvec4 texture(utexture2DArray tex, sampler sampl, vec3 P)
{
	return texture(usampler2DArray(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
ivec4 texture(itexture2DArray tex, sampler sampl, vec3 P)
{
	return texture(isampler2DArray(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
vec4 texture(textureCubeArray tex, sampler sampl, vec4 P, float bias)
{
	return texture(samplerCubeArray(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
uvec4 texture(utextureCubeArray tex, sampler sampl, vec4 P, float bias)
{
	return texture(usamplerCubeArray(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
ivec4 texture(itextureCubeArray tex, sampler sampl, vec4 P, float bias)
{
	return texture(isamplerCubeArray(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
vec4 texture(textureCubeArray tex, sampler sampl, vec4 P)
{
	return texture(samplerCubeArray(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
uvec4 texture(utextureCubeArray tex, sampler sampl, vec4 P)
{
	return texture(usamplerCubeArray(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
ivec4 texture(itextureCubeArray tex, sampler sampl, vec4 P)
{
	return texture(isamplerCubeArray(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
vec4 texture(texture1DArray tex, sampler sampl, vec2 P, float bias)
{
	return texture(sampler1DArray(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
uvec4 texture(utexture1DArray tex, sampler sampl, vec2 P, float bias)
{
	return texture(usampler1DArray(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
ivec4 texture(itexture1DArray tex, sampler sampl, vec2 P, float bias)
{
	return texture(isampler1DArray(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
vec4 texture(texture1DArray tex, sampler sampl, vec2 P)
{
	return texture(sampler1DArray(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
uvec4 texture(utexture1DArray tex, sampler sampl, vec2 P)
{
	return texture(usampler1DArray(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
ivec4 texture(itexture1DArray tex, sampler sampl, vec2 P)
{
	return texture(isampler1DArray(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
float texture(texture1DArray tex, samplerShadow sampl, vec3 P, float bias)
{
	return texture(sampler1DArrayShadow(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
float texture(texture1DArray tex, samplerShadow sampl, vec3 P)
{
	return texture(sampler1DArrayShadow(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
float texture(texture2DArray tex, samplerShadow sampl, vec4 P)
{
	return texture(sampler2DArrayShadow(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
float texture(textureCubeArray tex, samplerShadow sampl, vec4 P, float compare)
{
	return texture(samplerCubeArrayShadow(tex, sampl), P, compare);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
vec4 textureProj(texture1D tex, sampler sampl, vec2 P, float bias)
{
	return textureProj(sampler1D(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
uvec4 textureProj(utexture1D tex, sampler sampl, vec2 P, float bias)
{
	return textureProj(usampler1D(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
ivec4 textureProj(itexture1D tex, sampler sampl, vec2 P, float bias)
{
	return textureProj(isampler1D(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
vec4 textureProj(texture1D tex, sampler sampl, vec2 P)
{
	return textureProj(sampler1D(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
uvec4 textureProj(utexture1D tex, sampler sampl, vec2 P)
{
	return textureProj(usampler1D(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
ivec4 textureProj(itexture1D tex, sampler sampl, vec2 P)
{
	return textureProj(isampler1D(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
vec4 textureProj(texture1D tex, sampler sampl, vec4 P, float bias)
{
	return textureProj(sampler1D(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
uvec4 textureProj(utexture1D tex, sampler sampl, vec4 P, float bias)
{
	return textureProj(usampler1D(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
ivec4 textureProj(itexture1D tex, sampler sampl, vec4 P, float bias)
{
	return textureProj(isampler1D(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
vec4 textureProj(texture1D tex, sampler sampl, vec4 P)
{
	return textureProj(sampler1D(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
uvec4 textureProj(utexture1D tex, sampler sampl, vec4 P)
{
	return textureProj(usampler1D(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
ivec4 textureProj(itexture1D tex, sampler sampl, vec4 P)
{
	return textureProj(isampler1D(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
vec4 textureProj(texture2D tex, sampler sampl, vec3 P, float bias)
{
	return textureProj(sampler2D(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
uvec4 textureProj(utexture2D tex, sampler sampl, vec3 P, float bias)
{
	return textureProj(usampler2D(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
ivec4 textureProj(itexture2D tex, sampler sampl, vec3 P, float bias)
{
	return textureProj(isampler2D(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
vec4 textureProj(texture2D tex, sampler sampl, vec3 P)
{
	return textureProj(sampler2D(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
uvec4 textureProj(utexture2D tex, sampler sampl, vec3 P)
{
	return textureProj(usampler2D(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
ivec4 textureProj(itexture2D tex, sampler sampl, vec3 P)
{
	return textureProj(isampler2D(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
vec4 textureProj(texture2D tex, sampler sampl, vec4 P, float bias)
{
	return textureProj(sampler2D(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
uvec4 textureProj(utexture2D tex, sampler sampl, vec4 P, float bias)
{
	return textureProj(usampler2D(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
ivec4 textureProj(itexture2D tex, sampler sampl, vec4 P, float bias)
{
	return textureProj(isampler2D(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
vec4 textureProj(texture2D tex, sampler sampl, vec4 P)
{
	return textureProj(sampler2D(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
uvec4 textureProj(utexture2D tex, sampler sampl, vec4 P)
{
	return textureProj(usampler2D(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
ivec4 textureProj(itexture2D tex, sampler sampl, vec4 P)
{
	return textureProj(isampler2D(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
vec4 textureProj(texture3D tex, sampler sampl, vec4 P, float bias)
{
	return textureProj(sampler3D(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
uvec4 textureProj(utexture3D tex, sampler sampl, vec4 P, float bias)
{
	return textureProj(usampler3D(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
ivec4 textureProj(itexture3D tex, sampler sampl, vec4 P, float bias)
{
	return textureProj(isampler3D(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
vec4 textureProj(texture3D tex, sampler sampl, vec4 P)
{
	return textureProj(sampler3D(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
uvec4 textureProj(utexture3D tex, sampler sampl, vec4 P)
{
	return textureProj(usampler3D(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
ivec4 textureProj(itexture3D tex, sampler sampl, vec4 P)
{
	return textureProj(isampler3D(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
float textureProj(texture1D tex, samplerShadow sampl, vec4 P, float bias)
{
	return textureProj(sampler1DShadow(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
float textureProj(texture1D tex, samplerShadow sampl, vec4 P)
{
	return textureProj(sampler1DShadow(tex, sampl), P);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
float textureProj(texture2D tex, samplerShadow sampl, vec4 P, float bias)
{
	return textureProj(sampler2DShadow(tex, sampl), P, bias);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
float textureProj(texture2D tex, samplerShadow sampl, vec4 P)
{
	return textureProj(sampler2DShadow(tex, sampl), P);
}
#endif

vec4 textureLod(texture1D tex, sampler sampl, float P, float lod)
{
	return textureLod(sampler1D(tex, sampl), P, lod);
}

uvec4 textureLod(utexture1D tex, sampler sampl, float P, float lod)
{
	return textureLod(usampler1D(tex, sampl), P, lod);
}

ivec4 textureLod(itexture1D tex, sampler sampl, float P, float lod)
{
	return textureLod(isampler1D(tex, sampl), P, lod);
}

vec4 textureLod(texture2D tex, sampler sampl, vec2 P, float lod)
{
	return textureLod(sampler2D(tex, sampl), P, lod);
}

uvec4 textureLod(utexture2D tex, sampler sampl, vec2 P, float lod)
{
	return textureLod(usampler2D(tex, sampl), P, lod);
}

ivec4 textureLod(itexture2D tex, sampler sampl, vec2 P, float lod)
{
	return textureLod(isampler2D(tex, sampl), P, lod);
}

vec4 textureLod(texture3D tex, sampler sampl, vec3 P, float lod)
{
	return textureLod(sampler3D(tex, sampl), P, lod);
}

uvec4 textureLod(utexture3D tex, sampler sampl, vec3 P, float lod)
{
	return textureLod(usampler3D(tex, sampl), P, lod);
}

ivec4 textureLod(itexture3D tex, sampler sampl, vec3 P, float lod)
{
	return textureLod(isampler3D(tex, sampl), P, lod);
}

vec4 textureLod(textureCube tex, sampler sampl, vec3 P, float lod)
{
	return textureLod(samplerCube(tex, sampl), P, lod);
}

uvec4 textureLod(utextureCube tex, sampler sampl, vec3 P, float lod)
{
	return textureLod(usamplerCube(tex, sampl), P, lod);
}

ivec4 textureLod(itextureCube tex, sampler sampl, vec3 P, float lod)
{
	return textureLod(isamplerCube(tex, sampl), P, lod);
}

float textureLod(texture2D tex, samplerShadow sampl, vec3 P, float lod)
{
	return textureLod(sampler2DShadow(tex, sampl), P, lod);
}

float textureLod(texture1D tex, samplerShadow sampl, vec3 P, float lod)
{
	return textureLod(sampler1DShadow(tex, sampl), P, lod);
}

vec4 textureLod(texture1DArray tex, sampler sampl, vec2 P, float lod)
{
	return textureLod(sampler1DArray(tex, sampl), P, lod);
}

uvec4 textureLod(utexture1DArray tex, sampler sampl, vec2 P, float lod)
{
	return textureLod(usampler1DArray(tex, sampl), P, lod);
}

ivec4 textureLod(itexture1DArray tex, sampler sampl, vec2 P, float lod)
{
	return textureLod(isampler1DArray(tex, sampl), P, lod);
}

float textureLod(texture1DArray tex, samplerShadow sampl, vec3 P, float lod)
{
	return textureLod(sampler1DArrayShadow(tex, sampl), P, lod);
}

vec4 textureLod(texture2DArray tex, sampler sampl, vec3 P, float lod)
{
	return textureLod(sampler2DArray(tex, sampl), P, lod);
}

uvec4 textureLod(utexture2DArray tex, sampler sampl, vec3 P, float lod)
{
	return textureLod(usampler2DArray(tex, sampl), P, lod);
}

ivec4 textureLod(itexture2DArray tex, sampler sampl, vec3 P, float lod)
{
	return textureLod(isampler2DArray(tex, sampl), P, lod);
}

vec4 textureLod(textureCubeArray tex, sampler sampl, vec4 P, float lod)
{
	return textureLod(samplerCubeArray(tex, sampl), P, lod);
}

uvec4 textureLod(utextureCubeArray tex, sampler sampl, vec4 P, float lod)
{
	return textureLod(usamplerCubeArray(tex, sampl), P, lod);
}

ivec4 textureLod(itextureCubeArray tex, sampler sampl, vec4 P, float lod)
{
	return textureLod(isamplerCubeArray(tex, sampl), P, lod);
}

vec4 textureProjLod(texture1D tex, sampler sampl, vec2 P, float lod)
{
	return textureProjLod(sampler1D(tex, sampl), P, lod);
}

uvec4 textureProjLod(utexture1D tex, sampler sampl, vec2 P, float lod)
{
	return textureProjLod(usampler1D(tex, sampl), P, lod);
}

ivec4 textureProjLod(itexture1D tex, sampler sampl, vec2 P, float lod)
{
	return textureProjLod(isampler1D(tex, sampl), P, lod);
}

vec4 textureProjLod(texture1D tex, sampler sampl, vec4 P, float lod)
{
	return textureProjLod(sampler1D(tex, sampl), P, lod);
}

uvec4 textureProjLod(utexture1D tex, sampler sampl, vec4 P, float lod)
{
	return textureProjLod(usampler1D(tex, sampl), P, lod);
}

ivec4 textureProjLod(itexture1D tex, sampler sampl, vec4 P, float lod)
{
	return textureProjLod(isampler1D(tex, sampl), P, lod);
}

vec4 textureProjLod(texture2D tex, sampler sampl, vec3 P, float lod)
{
	return textureProjLod(sampler2D(tex, sampl), P, lod);
}

uvec4 textureProjLod(utexture2D tex, sampler sampl, vec3 P, float lod)
{
	return textureProjLod(usampler2D(tex, sampl), P, lod);
}

ivec4 textureProjLod(itexture2D tex, sampler sampl, vec3 P, float lod)
{
	return textureProjLod(isampler2D(tex, sampl), P, lod);
}

vec4 textureProjLod(texture2D tex, sampler sampl, vec4 P, float lod)
{
	return textureProjLod(sampler2D(tex, sampl), P, lod);
}

uvec4 textureProjLod(utexture2D tex, sampler sampl, vec4 P, float lod)
{
	return textureProjLod(usampler2D(tex, sampl), P, lod);
}

ivec4 textureProjLod(itexture2D tex, sampler sampl, vec4 P, float lod)
{
	return textureProjLod(isampler2D(tex, sampl), P, lod);
}

vec4 textureProjLod(texture3D tex, sampler sampl, vec4 P, float lod)
{
	return textureProjLod(sampler3D(tex, sampl), P, lod);
}

uvec4 textureProjLod(utexture3D tex, sampler sampl, vec4 P, float lod)
{
	return textureProjLod(usampler3D(tex, sampl), P, lod);
}

ivec4 textureProjLod(itexture3D tex, sampler sampl, vec4 P, float lod)
{
	return textureProjLod(isampler3D(tex, sampl), P, lod);
}

float textureProjLod(texture1D tex, samplerShadow sampl, vec4 P, float lod)
{
	return textureProjLod(sampler1DShadow(tex, sampl), P, lod);
}

float textureProjLod(texture2D tex, samplerShadow sampl, vec4 P, float lod)
{
	return textureProjLod(sampler2DShadow(tex, sampl), P, lod);
}

#if defined(ANKI_FRAGMENT_SHADER)
vec4 textureGrad(texture1D tex, sampler sampl, float P, float dPdx, float dPdy)
{
	return textureGrad(sampler1D(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
uvec4 textureGrad(utexture1D tex, sampler sampl, float P, float dPdx, float dPdy)
{
	return textureGrad(usampler1D(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
ivec4 textureGrad(itexture1D tex, sampler sampl, float P, float dPdx, float dPdy)
{
	return textureGrad(isampler1D(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
vec4 textureGrad(texture2D tex, sampler sampl, vec2 P, vec2 dPdx, vec2 dPdy)
{
	return textureGrad(sampler2D(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
uvec4 textureGrad(utexture2D tex, sampler sampl, vec2 P, vec2 dPdx, vec2 dPdy)
{
	return textureGrad(usampler2D(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
ivec4 textureGrad(itexture2D tex, sampler sampl, vec2 P, vec2 dPdx, vec2 dPdy)
{
	return textureGrad(isampler2D(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
vec4 textureGrad(texture3D tex, sampler sampl, vec3 P, vec3 dPdx, vec3 dPdy)
{
	return textureGrad(sampler3D(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
uvec4 textureGrad(utexture3D tex, sampler sampl, vec3 P, vec3 dPdx, vec3 dPdy)
{
	return textureGrad(usampler3D(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
ivec4 textureGrad(itexture3D tex, sampler sampl, vec3 P, vec3 dPdx, vec3 dPdy)
{
	return textureGrad(isampler3D(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
vec4 textureGrad(textureCube tex, sampler sampl, vec3 P, vec3 dPdx, vec3 dPdy)
{
	return textureGrad(samplerCube(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
uvec4 textureGrad(utextureCube tex, sampler sampl, vec3 P, vec3 dPdx, vec3 dPdy)
{
	return textureGrad(usamplerCube(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
ivec4 textureGrad(itextureCube tex, sampler sampl, vec3 P, vec3 dPdx, vec3 dPdy)
{
	return textureGrad(isamplerCube(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
float textureGrad(texture1D tex, samplerShadow sampl, vec3 P, float dPdx, float dPdy)
{
	return textureGrad(sampler1DShadow(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
vec4 textureGrad(texture1DArray tex, sampler sampl, vec2 P, float dPdx, float dPdy)
{
	return textureGrad(sampler1DArray(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
uvec4 textureGrad(utexture1DArray tex, sampler sampl, vec2 P, float dPdx, float dPdy)
{
	return textureGrad(usampler1DArray(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
ivec4 textureGrad(itexture1DArray tex, sampler sampl, vec2 P, float dPdx, float dPdy)
{
	return textureGrad(isampler1DArray(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
vec4 textureGrad(texture2DArray tex, sampler sampl, vec3 P, vec2 dPdx, vec2 dPdy)
{
	return textureGrad(sampler2DArray(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
uvec4 textureGrad(utexture2DArray tex, sampler sampl, vec3 P, vec2 dPdx, vec2 dPdy)
{
	return textureGrad(usampler2DArray(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
ivec4 textureGrad(itexture2DArray tex, sampler sampl, vec3 P, vec2 dPdx, vec2 dPdy)
{
	return textureGrad(isampler2DArray(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
float textureGrad(texture1DArray tex, samplerShadow sampl, vec3 P, float dPdx, float dPdy)
{
	return textureGrad(sampler1DArrayShadow(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
float textureGrad(texture2D tex, samplerShadow sampl, vec3 P, vec2 dPdx, vec2 dPdy)
{
	return textureGrad(sampler2DShadow(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
float textureGrad(textureCube tex, samplerShadow sampl, vec4 P, vec3 dPdx, vec3 dPdy)
{
	return textureGrad(samplerCubeShadow(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
float textureGrad(texture2DArray tex, samplerShadow sampl, vec4 P, vec2 dPdx, vec2 dPdy)
{
	return textureGrad(sampler2DArrayShadow(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
vec4 textureGrad(textureCubeArray tex, sampler sampl, vec4 P, vec3 dPdx, vec3 dPdy)
{
	return textureGrad(samplerCubeArray(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
uvec4 textureGrad(utextureCubeArray tex, sampler sampl, vec4 P, vec3 dPdx, vec3 dPdy)
{
	return textureGrad(usamplerCubeArray(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
ivec4 textureGrad(itextureCubeArray tex, sampler sampl, vec4 P, vec3 dPdx, vec3 dPdy)
{
	return textureGrad(isamplerCubeArray(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
vec4 textureProjGrad(texture1D tex, sampler sampl, vec2 P, float dPdx, float dPdy)
{
	return textureProjGrad(sampler1D(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
uvec4 textureProjGrad(utexture1D tex, sampler sampl, vec2 P, float dPdx, float dPdy)
{
	return textureProjGrad(usampler1D(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
ivec4 textureProjGrad(itexture1D tex, sampler sampl, vec2 P, float dPdx, float dPdy)
{
	return textureProjGrad(isampler1D(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
vec4 textureProjGrad(texture1D tex, sampler sampl, vec4 P, float dPdx, float dPdy)
{
	return textureProjGrad(sampler1D(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
uvec4 textureProjGrad(utexture1D tex, sampler sampl, vec4 P, float dPdx, float dPdy)
{
	return textureProjGrad(usampler1D(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
ivec4 textureProjGrad(itexture1D tex, sampler sampl, vec4 P, float dPdx, float dPdy)
{
	return textureProjGrad(isampler1D(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
vec4 textureProjGrad(texture2D tex, sampler sampl, vec3 P, vec2 dPdx, vec2 dPdy)
{
	return textureProjGrad(sampler2D(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
uvec4 textureProjGrad(utexture2D tex, sampler sampl, vec3 P, vec2 dPdx, vec2 dPdy)
{
	return textureProjGrad(usampler2D(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
ivec4 textureProjGrad(itexture2D tex, sampler sampl, vec3 P, vec2 dPdx, vec2 dPdy)
{
	return textureProjGrad(isampler2D(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
vec4 textureProjGrad(texture2D tex, sampler sampl, vec4 P, vec2 dPdx, vec2 dPdy)
{
	return textureProjGrad(sampler2D(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
uvec4 textureProjGrad(utexture2D tex, sampler sampl, vec4 P, vec2 dPdx, vec2 dPdy)
{
	return textureProjGrad(usampler2D(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
ivec4 textureProjGrad(itexture2D tex, sampler sampl, vec4 P, vec2 dPdx, vec2 dPdy)
{
	return textureProjGrad(isampler2D(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
vec4 textureProjGrad(texture3D tex, sampler sampl, vec4 P, vec3 dPdx, vec3 dPdy)
{
	return textureProjGrad(sampler3D(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
uvec4 textureProjGrad(utexture3D tex, sampler sampl, vec4 P, vec3 dPdx, vec3 dPdy)
{
	return textureProjGrad(usampler3D(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
ivec4 textureProjGrad(itexture3D tex, sampler sampl, vec4 P, vec3 dPdx, vec3 dPdy)
{
	return textureProjGrad(isampler3D(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
float textureProjGrad(texture1D tex, samplerShadow sampl, vec4 P, float dPdx, float dPdy)
{
	return textureProjGrad(sampler1DShadow(tex, sampl), P, dPdx, dPdy);
}
#endif

#if defined(ANKI_FRAGMENT_SHADER)
float textureProjGrad(texture2D tex, samplerShadow sampl, vec4 P, vec2 dPdx, vec2 dPdy)
{
	return textureProjGrad(sampler2DShadow(tex, sampl), P, dPdx, dPdy);
}
#endif
