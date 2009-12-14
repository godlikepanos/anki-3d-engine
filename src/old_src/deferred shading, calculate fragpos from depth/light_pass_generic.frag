uniform sampler2D normal_rt, diffuse_rt, specular_rt, depth_rt;
uniform vec2 planes;

#if defined(_PROJECTED_TXTR_LIGHT_)
uniform sampler2D light_txtr;
#endif

#if defined(_PROJECTED_TXTR_LIGHT_SHADOW_)
uniform sampler2DShadow shadow_depth_map;
uniform float shadow_resolution;
#endif

varying vec2 txtr_coord;
varying vec3 vpos;


/*
=======================================================================================================================================
Discard                                                                                                                               =
what to do if the fragment shouldnt be processed                                                                                      =
=======================================================================================================================================
*/
void Discard()
{
	//gl_FragData[0] = vec4(1.0, 0.0, 1.0, 1.0 );
	discard;
}


/*
=======================================================================================================================================
FragPosVSpace                                                                                                                         =
calc frag pos in view space                                                                                                           =
=======================================================================================================================================
*/
vec3 FragPosVSpace()
{
	float _depth = texture2D( depth_rt, txtr_coord ).r;

	if( _depth == 1.0 ) Discard();

	vec3 _frag_pos_vspace;
	vec3 _vposn = normalize(vpos);
	_frag_pos_vspace.z = -planes.y/(planes.x+_depth);
	_frag_pos_vspace.xy = _vposn.xy/_vposn.z*_frag_pos_vspace.z;
	return _frag_pos_vspace;
}


/*
=======================================================================================================================================
Attenuation                                                                                                                           =
=======================================================================================================================================
*/
float Attenuation( in float _frag_light_dist )
{
	float _inv_light_radius = gl_LightSource[0].position.w;
	return clamp(1.0 - _inv_light_radius * sqrt(_frag_light_dist), 0.0, 1.0);
	//return 1.0 - _frag_light_dist * _inv_light_radius;
}


/*
=======================================================================================================================================
PCF                                                                                                                                   =
it returns a blured shadow                                                                                                            =
=======================================================================================================================================
*/
#if defined(_PROJECTED_TXTR_LIGHT_SHADOW_)
float PCF_Off( in vec3 _shadow_uv )
{
	return shadow2D(shadow_depth_map, _shadow_uv ).r;
}


float PCF_Low( in vec3 _shadow_uv )
{
	float _shadow_col = shadow2D(shadow_depth_map, _shadow_uv ).r;
	float _map_scale = 1.0 / shadow_resolution;

	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3( _map_scale,  _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3( _map_scale, -_map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3( _map_scale,  	     0.0, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3(-_map_scale,  _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3(-_map_scale, -_map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3(-_map_scale,  	     0.0, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3(        0.0,  _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3(        0.0, -_map_scale, 0.0)).r;
	_shadow_col /= 9.0;

	return _shadow_col;
}


float PCF_Medium( in vec3 _shadow_uv )
{
	float _shadow_col = shadow2D(shadow_depth_map, _shadow_uv ).r;
	float _map_scale = 1.0 / shadow_resolution;

	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3( _map_scale,  _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3( _map_scale, -_map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3( _map_scale,         0.0, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3(-_map_scale,  _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3(-_map_scale, -_map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3(-_map_scale,         0.0, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3(        0.0,  _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3(        0.0, -_map_scale, 0.0)).r;

	_map_scale *= 2.0;

	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3( _map_scale,  _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3( _map_scale, -_map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3( _map_scale,         0.0, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3(-_map_scale,  _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3(-_map_scale, -_map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3(-_map_scale,         0.0, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3(        0.0,  _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3(        0.0, -_map_scale, 0.0)).r;

	_shadow_col *= 0.058823529; // aka: _shadow_col /= 17.0;
	return _shadow_col;
}


float PCF_High( in vec3 _shadow_uv )
{
	float _shadow_col = shadow2D(shadow_depth_map, _shadow_uv ).r;
	float _map_scale = 1.0 / shadow_resolution;

	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3( _map_scale,  _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3( _map_scale, -_map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3( _map_scale,  	     0.0, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3(-_map_scale,  _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3(-_map_scale, -_map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3(-_map_scale,  	     0.0, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3(        0.0,  _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3(        0.0, -_map_scale, 0.0)).r;


	float _map_scale_2 = 2.0 * _map_scale;

	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3(_map_scale_2, _map_scale_2, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3(_map_scale, _map_scale_2, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3(0.0, _map_scale_2, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3(-_map_scale, _map_scale_2, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3(-_map_scale_2, _map_scale_2, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3(-_map_scale_2, _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3(-_map_scale_2, 0.0, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3(-_map_scale_2, -_map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3(-_map_scale_2, -_map_scale_2, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3(-_map_scale, -_map_scale_2, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3(0.0, -_map_scale_2, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3(_map_scale, -_map_scale_2, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3(_map_scale_2, -_map_scale_2, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3(_map_scale_2, -_map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3(_map_scale_2, 0.0, 0.0)).r;
	_shadow_col += shadow2D(shadow_depth_map, _shadow_uv.xyz + vec3(_map_scale_2, _map_scale, 0.0)).r;

	_shadow_col /= 25.0;
	return _shadow_col;
}
#endif


/*
=======================================================================================================================================
main                                                                                                                                  =
=======================================================================================================================================
*/
void main()
{
	// get frag pos in view space
	vec3 _frag_pos_vspace = FragPosVSpace();

	// get lambert term
	vec3 _light_pos_eyespace = gl_LightSource[0].position.xyz;
	vec3 _light_frag_vec = _light_pos_eyespace - _frag_pos_vspace;

	float _frag_light_dist = dot( _light_frag_vec, _light_frag_vec ); // instead of using normalize(_frag_light_dist) we brake the operation...
	vec3 _light_dir = _light_frag_vec * inversesqrt(_frag_light_dist); // ...because we want frag_light_dist for the calc of the attenuation

	// read the render targets
	vec3 _normal = ( texture2D( normal_rt, txtr_coord ).rgb - 0.5 ) * 2.0;  // read the normal and convert from [0,1] to [-1,1]

	// the lambert term
	float _lambert_term = dot( _normal, _light_dir );

	if( _lambert_term < 0.0 ) Discard();

	// diffuce lighting
	vec3 _diffuse = texture2D( diffuse_rt, txtr_coord ).rgb;
	_diffuse = (_diffuse * gl_LightSource[0].diffuse.rgb);
	vec3 _color = _diffuse * _lambert_term;

	// specular lighting
	vec4 _specular_mix = texture2D( specular_rt, txtr_coord );
	vec3 _specular = _specular_mix.xyz;
	float _shininess = _specular_mix.w * 128.0;
	vec3 _eye_vec = normalize( -_frag_pos_vspace );
	vec3 _h = normalize( _light_dir + _eye_vec );
	float _spec_intensity = pow(max(0.0, dot(_normal, _h)), _shininess);
	_color += _specular * vec3(gl_LightSource[0].specular) * (_spec_intensity * _lambert_term);


#if defined(_POINT_LIGHT_)

	gl_FragData[0] = vec4( _color * Attenuation(_frag_light_dist), 1.0 );

#elif defined(_PROJECTED_TXTR_LIGHT_)

#if defined(_PROJECTED_TXTR_LIGHT_NO_SHADOW_)

	vec4 _txtr_coord2 = gl_TextureMatrix[0] * vec4(_frag_pos_vspace, 1.0);
	vec3 _txtr_coord3 = _txtr_coord2.xyz / _txtr_coord2.w;

	if
	(
		_txtr_coord2.w > 0.0 &&
		_txtr_coord2.w < 1.0/gl_LightSource[0].position.w &&
		_txtr_coord3.x >= 0.0 &&
		_txtr_coord3.x <= 1.0 &&
		_txtr_coord3.y >= 0.0 &&
		_txtr_coord3.y <= 1.0
	)
	{
		vec3 _texel = texture2DProj( light_txtr, _txtr_coord2.xyz ).rgb;
		vec3 _texel_with_lambert = _texel * _lambert_term; // make that mul because we dont want the faces that are not facing the light
																											 // ... to have the texture applied to them
		float _att = Attenuation(_frag_light_dist);
		gl_FragData[0] = vec4( _color * _texel_with_lambert * _att, 1.0 );
	}
	else
		Discard();

#elif defined(_PROJECTED_TXTR_LIGHT_SHADOW_)
	vec4 _txtr_coord2 = gl_TextureMatrix[0] * vec4(_frag_pos_vspace, 1.0);
	vec3 _shadow_uv = _txtr_coord2.xyz / _txtr_coord2.w;

	if
	(
		_txtr_coord2.w > 0.0 && // cat behind
		_txtr_coord2.w < 1.0/gl_LightSource[0].position.w && // cat the far front
		_shadow_uv.x >= 0.0 && // cat left
		_shadow_uv.x <= 1.0 && // cat right
		_shadow_uv.y >= 0.0 && // cat down
		_shadow_uv.y <= 1.0   // cat up
	)
	{
		float _shadow_color = PCF_Medium( _shadow_uv );
		float _att = Attenuation(_frag_light_dist);
		vec3 _texel = texture2DProj( light_txtr, _txtr_coord2.xyz ).rgb;
		gl_FragData[0] = vec4(_texel * _color * (_shadow_color * _att), 1.0);
	}
	else
		Discard();
#endif

#endif

	//gl_FragData[0] = vec4( _normal*0.5 + 0.5, 1.0 );
}
