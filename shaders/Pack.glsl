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
