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

// If not defined fake it to stop some compilers from complaining
#ifndef USE_MRT
#	define USE_MRT 1
#endif

#define MAX_SPECULARITY 128.0

// Populate the G buffer
void writeGBuffer(
	in vec3 diffColor, in vec3 normal, in float specColor, in float specPower,
#if USE_MRT
	out vec4 fai0, out vec4 fai1
#else
	out highp uvec2 fai0
#endif
	)
{
	vec3 unorm = normal * 0.5 + 0.5;
#if USE_MRT
	fai0 = vec4(diffColor, specColor);
	fai1 = vec4(unorm.xyz, specPower / MAX_SPECULARITY);
#else
	fai0.x = packUnorm4x8(vec4(diffColor, specColor));
	fai0.y = packUnorm4x8(vec4(unorm, specPower / MAX_SPECULARITY));
#endif
}

// Read from the G buffer
void readGBuffer(
#if USE_MRT
	in sampler2D fai0, in sampler2D fai1,
#else
	in highp usampler2D fai0,
#endif
	in vec2 texCoord,
	out vec3 diffColor, out vec3 normal, out float specColor, 
	out float specPower)
{
#if USE_MRT
	vec4 comp = texture(fai0, texCoord);
	diffColor = comp.rgb;
	specColor = comp.a;

	comp = texture(fai1, texCoord);
	normal = normalize(comp.xyz * 2.0 - 1.0);
	specPower = comp.w * MAX_SPECULARITY;
#else
	highp uvec2 all_ = texture(fai0, texCoord).rg;

	vec4 v = unpackUnorm4x8(all_[0]);
	diffColor = v.rgb;
	specColor = v.a;

	v = unpackUnorm4x8(all_[1]);
	normal = normalize(v.xyz * 2.0 - 1.0);
	specPower = v.w * MAX_SPECULARITY;
#endif
}

// Read only normal from G buffer
void readNormalFromGBuffer(
#if USE_MRT
	in sampler2D fai1,
#else
	in highp usampler2D fai0,
#endif
	in vec2 texCoord,
	out vec3 normal)
{
#if USE_MRT
	normal = normalize(texture(fai1, texCoord).xyz);
#else
	vec4 v = unpackUnorm4x8(texture(fai0, texCoord).g);
	normal = normalize(v.xyz * 2.0 - 1.0);
#endif
}
