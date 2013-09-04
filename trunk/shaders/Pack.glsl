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

vec2 encodeUnormFloatToVec2(in float f)
{
	vec2 vec = vec2(1.0, 65025.0) * f;
	return vec2(vec.x, fract(vec.y));
	//return unpackSnorm2x16(floatBitsToUint(f));
}

float decodeVec2ToUnormFloat(in vec2 vec)
{
	//return uintBitsToFloat(packSnorm2x16(vec));
	return dot(vec, vec2(1.0, 1.0 / 65025.0));
}

void packAndWriteNormal(in vec3 normal, out vec4 fai)
{
#if 1
	vec3 unorm = normal * 0.5 + 0.5;
	fai = vec4(unorm.xyz, 0.0);
#endif

#if 0
	vec3 unorm = normal * 0.5 + 0.5;
	float maxc = max(max(unorm.x, unorm.y), unorm.z);
	fai = vec4(unorm.xyz * maxc, maxc);
#endif

#if 0
	vec2 enc = packNormal(normal) * 0.5 + 0.5;
	fai = vec4(encodeUnormFloatToVec2(enc.x), encodeUnormFloatToVec2(enc.y));
#endif

#if 0
	vec2 unorm = normal.xy * 0.5 + 0.5;
	vec2 x = encodeUnormFloatToVec2(unorm.x);
	vec2 y = encodeUnormFloatToVec2(unorm.y);
	fai = vec4(x, y);
#endif
	//fai = packNormal(normal_);
}

vec3 readAndUnpackNormal(in sampler2D fai, in vec2 texCoord)
{
#if 1
	return normalize(texture(fai, texCoord).xyz * 2.0 - 1.0);
#endif

#if 0
	vec4 enc = texture(fai, texCoord);
	return normalize(enc.xyz * (2.0 / enc.w) - 1.0);
#endif

#if 0
	vec4 enc = texture(fai, texCoord);
	vec2 enc2 = vec2(decodeVec2ToUnormFloat(enc.xy), 
		decodeVec2ToUnormFloat(enc.zw)) * 2.0 - 1.0;
	return unpackNormal(enc2);
#endif

#if 0
	vec4 enc = texture(fai, texCoord);
	vec2 xy = vec2(decodeVec2ToUnormFloat(enc.xy),
		decodeVec2ToUnormFloat(enc.zw)) * 2.0 - 1.0;
	float z = sqrt(1.0 - dot(xy, xy)) ;
	vec3 snorm = vec3(xy, z);
	return snorm;
#endif
	//unpackNormal(texture(fai_, texCoord_).rg)
}

#define MAX_SPECULARITY 128.0

/// Pack specular stuff
/// @param c The specular component. c.x is the color in grayscale and c.y the 
///          specularity
#define packSpecular_DEFINED
float packSpecular(in vec2 c)
{
	return round(c[0] * 15.0) * (16.0 / 255.0) 
		+ round(c[1] / MAX_SPECULARITY * 15.0) / 255.0;
}

/// Unpack specular
vec2 unpackSpecular(in float f)
{
	float r = floor(f * (255.0 / 16.0));

	return vec2(
		r / 15.0,
		f * (255.0 * MAX_SPECULARITY / 15.0) 
		- r * (16.0 * MAX_SPECULARITY / 15.0));
}

#if GL_ES
uint packUnorm4x8(in vec4 v)
{
	vec4 value = clamp(v, 0.0, 1.0) * 255.0;

	return uint(value.x) | (uint(value.y) << 8) | (uint(value.z) << 16) |
		(uint(value.w) << 24);
}
 
vec4 unpackUnorm4x8(in uint u)
{
	vec4 value;

	value.x = float(u & 0xffU);
	value.y = float((u >> 8) & 0xffU);
	value.z = float((u >> 16) & 0xffU);
	value.w = float((u >> 24) & 0xffU);

	return value * (1.0 / 255.0);
}
#endif

// Populate the G buffer
void writeGBuffer(
	in vec3 diffColor, in vec3 normal, in float specColor, in float specPower,
	out vec4 fai0
#if USE_MRT
	,out vec4 fai1
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
	uvec2 all_ = texture(fai0, texCoord).rg;

	vec4 v = unpackUnorm4x8(all_[0]);
	diffColor = v.rgb;
	specColor = v.a * MAX_SPECULARITY;

	v = unpackUnorm4x8(all_[1]);
	normal = normalize(v.xyz * 2.0 - 1.0);
	specPower = v.xyz;
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
	normal = normalize(readAndUnpackNormal(fai1, texCoord).xyz);
#else
	vec4 v = unpackUnorm4x8(texture(fai0, texCoord).g);
	normal = normalize(v.xyz * 2.0 - 1.0);
#endif
}
