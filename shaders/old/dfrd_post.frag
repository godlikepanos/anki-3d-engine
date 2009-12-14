#pragma optimize(off)

uniform sampler2D normal_buf, diffuse_buf, specular_buf;
uniform sampler2D depth_buf;
uniform vec2 planes;
uniform vec3 light_pos_eyespace;
varying vec3 vpos1;

vec3 Color2Normal( in vec3 _col )
{
	vec3 _normal;
	_normal.x = _col.r - 0.5;
	_normal.y = _col.g - 0.5;
	_normal.z = _col.b - 0.5;
	_normal = _normal * 2.0;
	return _normal;
}

void main()
{
	vec3 _normal   = Color2Normal( texture2D( normal_buf, gl_TexCoord[0].st ).rgb );
	vec4 _diffuse  = texture2D( diffuse_buf, gl_TexCoord[0].st );
	vec4 _specular = texture2D( specular_buf, gl_TexCoord[0].st );
	float _depth   = texture2D( depth_buf, gl_TexCoord[0].st ).r;


  vec3 view = normalize(vpos1);

	vec3 _pos;
	_pos.z = -planes.y/(planes.x+_depth);
	_pos.xy = view.xy/view.z*_pos.z;


	vec3 _light_dir = /*normalize*/( light_pos_eyespace - _pos );

	float invRadius = 1.0 / 4.0;

	float distSqr = dot( _light_dir, _light_dir );
	float att = clamp(1.0 - invRadius * sqrt(distSqr), 0.0, 1.0);
	vec3 L = _light_dir * inversesqrt(distSqr);


	float _lambert_term = dot( _normal, L );
	_diffuse = _diffuse * gl_LightSource[0].diffuse;
	vec4 _color = _diffuse * _lambert_term;

	gl_FragColor = _color * att;
	//gl_FragColor = vec4(_diffuse) ;
}

