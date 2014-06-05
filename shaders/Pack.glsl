// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

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

#define MAX_SPECULARITY 128.0

// Populate the G buffer
void writeGBuffer(
	in vec3 diffColor, in vec3 normal, in float specColor, in float specPower,
	out vec4 fai0, out vec4 fai1)
{
	vec3 unorm = normal * 0.5 + 0.5;
	fai0 = vec4(diffColor, specPower / MAX_SPECULARITY);
	fai1 = vec4(unorm.xyz, specColor);
}

// Read from the G buffer
void readGBuffer(
	in sampler2D fai0, in sampler2D fai1,
	in vec2 texCoord,
	out vec3 diffColor, out vec3 normal, out float specColor, 
	out float specPower)
{
	vec4 comp = textureRt(fai0, texCoord);
	diffColor = comp.rgb;
	specPower = comp.w * MAX_SPECULARITY;

	comp = textureRt(fai1, texCoord);
	normal = normalize(comp.xyz * 2.0 - 1.0);
	specColor = comp.a;
}

// Read only normal from G buffer
void readNormalFromGBuffer(
	in sampler2D fai1,
	in vec2 texCoord,
	out vec3 normal)
{
	normal = normalize(textureRt(fai1, texCoord).xyz * 2.0 - 1.0);
}

// Read only normal and specular color from G buffer
void readNormalSpecularColorFromGBuffer(
	in sampler2D fai1,
	in vec2 texCoord,
	out vec3 normal,
	out float specColor)
{
	vec4 comp = textureRt(fai1, texCoord);
	normal = normalize(comp.xyz * 2.0 - 1.0);
	specColor = comp.a;
}
