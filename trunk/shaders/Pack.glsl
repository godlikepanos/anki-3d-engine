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
float packSpecular(in vec2 c)
{
	return round(c[0] * 15.0) * 16.0 / 255.0 
		+ round(c[1] / MAX_SPECULARITY * 15.0) / 255.0;
}

/// Unpack specular
vec2 unpackSpecular(in float f)
{
	float r = floor(f * 255.0 / 16.0);

	return vec2(
		r / 15.0,
		f * 255.0 * MAX_SPECULARITY / 15.0 - r * 16.0 * MAX_SPECULARITY / 15.0);
}
