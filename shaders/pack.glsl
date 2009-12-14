vec2 PackNormal( in vec3 _normal )
{
    /*original code:
    const float _scale = 1.7777;
    vec2 _enc = _normal.xy / (_normal.z+1.0);
    _enc /= _scale;
    _enc = _enc*0.5 + 0.5;
    return _enc;*/

    const float _scale = 1.7777;
    float scalar1 = (_normal.z+1.0)*(_scale*2.0);
    return _normal.xy / scalar1 + 0.5;
}


vec3 UnpackNormal( in vec2 _enc )
{
	/*original code:
	float _scale = 1.7777;
	vec3 _nn = vec3( _enc*vec2(2.0*_scale, 2.0*_scale) + vec2(-_scale, -_scale), 1.0 );
	float _g = 2.0 / dot(_nn.xyz, _nn.xyz);
	vec3 _normal;
	_normal.xy = _g * _nn.xy;
	_normal.z = _g - 1.0;
	return _normal;*/

	const float _scale = 1.7777;
	vec2 _nn = _enc*(2.0*_scale) - _scale;
	float _g = 2.0 / (dot(_nn.xy, _nn.xy) + 1.0);
	vec3 _normal;
	_normal.xy = _g * _nn.xy;
	_normal.z = _g - 1.0;
	return _normal;
}
