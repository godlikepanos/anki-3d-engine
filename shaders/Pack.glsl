/// Pack 3D normal to 2D vector
/// See the clean code in commends in revision < r467
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
	return round(c[0] * 15.0) * 16.0 / 255.0 
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

/// Pack a vec4 to float
/// @node The components of v should be inside [0.0, 1.0]
float pack4x8ToFloat(in vec4 v)
{
	const vec4 unshift = vec4(
		1.0 / (256.0 * 256.0 * 256.0), 
		1.0 / (256.0 * 256.0), 
		1.0 / 256.0, 1.0);
	return dot(v, unshift);
}

/// Unpack a float to vec4
vec4 unpackFloatTo4x8(in float val) 
{
	const vec4 bitSh = vec4(
		256.0 * 256.0 * 256.0, 
		256.0* 256.0, 
		256.0, 
		1.0);
	const vec4 bitMsk = vec4(0.0, 1.0 / 256.0, 1.0 / 256.0, 1.0 / 256.0);

	vec4 result = fract(val * bitSh);
	result -= result.xxyz * bitMsk;
	return result;
}
