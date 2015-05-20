// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SHADERS_PACK_GLSL
#define ANKI_SHADERS_PACK_GLSL

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
	return normal;
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
	float temp = c * (255.0 / 16.0);
	float a = floor(temp);
	float b = fract(temp);
	return vec2(a, b) * vec2(1.0 / 15.0, 16.0 / 15.0);
}

// Populate the G buffer
void writeGBuffer(
	in vec3 diffColor,
	in vec3 normal,
	in vec3 specColor,
	in float roughness,
	out vec4 fai0,
	out vec4 fai1,
	out vec4 fai2)
{
	fai0 = vec4(diffColor, 0.0);
	fai1 = vec4(specColor, roughness);
	fai2 = vec4(normal * 0.5 + 0.5, 0.0);
}

// Read from the G buffer
void readGBuffer(
	in sampler2D fai0,
	in sampler2D fai1,
	in sampler2D fai2,
	in vec2 texCoord,
	out vec3 diffColor,
	out vec3 normal,
	out float specColor,
	out float roughness)
{
	vec4 comp = textureLod(fai0, texCoord, 0.0);
	diffColor = comp.rgb;

	comp = textureLod(fai1, texCoord, 0.0);
	specColor = comp.r; // XXX
	roughness = comp.a;

	normal = textureLod(fai2, texCoord, 0.0).rgb;
	normal = normalize(normal * 2.0 - 1.0);
}

// Read only normal from G buffer
void readNormalFromGBuffer(
	in sampler2D fai2,
	in vec2 texCoord,
	out vec3 normal)
{
	normal = normalize(textureLod(fai2, texCoord, 0.0).rgb * 2.0 - 1.0);
}

// Read only normal and specular color from G buffer
void readNormalSpecularColorFromGBuffer(
	in sampler2D fai1,
	in sampler2D fai2,
	in vec2 texCoord,
	out vec3 normal,
	out float specColor)
{
	normal = normalize(textureLod(fai2, texCoord, 0.0).rgb * 2.0 - 1.0);
	specColor = textureLod(fai1, texCoord, 0.0).r;
}

#endif
