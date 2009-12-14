varying vec3 txtr_coord;
uniform sampler2D normal_buf;

vec3 Color2Vec( in vec3 _col )
{
	vec3 _vec3 = _col;
	_col.x -= 0.5;
	_col.y -= 0.5;
	_col.z -= 0.5;
	_vec3 = _vec3 * 2.0;
	return _vec3;
}


void main()
{
	vec4 _normal = texture2DProj( normal_buf, txtr_coord );

	gl_FragColor = _normal * gl_Color;
}

