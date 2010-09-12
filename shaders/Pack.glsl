/**
 * Pack 3D normal to 2D vector
 */
vec2 packNormal( in vec3 _normal )
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


/**
 * Reverse the packNormal
 */
vec3 unpackNormal(in vec2 _enc_)
{
	const float _scale_ = 1.7777;
	vec2 _nn_ = _enc_ * (2.0 * _scale_) - _scale_;
	float _g_ = 2.0 / (dot(_nn_.xy, _nn_.xy) + 1.0);
	vec3 _normal_;
	_normal_.xy = _g_ * _nn_.xy;
	_normal_.z = _g_ - 1.0;
	return _normal_;
}
