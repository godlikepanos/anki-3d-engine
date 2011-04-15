/// Pack 3D normal to 2D vector
vec2 packNormal(in vec3 normal)
{
    /*original code:
    const float SCALE = 1.7777;
    vec2 _enc = _normal.xy / (_normal.z+1.0);
    _enc /= SCALE;
    _enc = _enc*0.5 + 0.5;
    return _enc;*/

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
