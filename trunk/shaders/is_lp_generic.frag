uniform sampler2D ms_normal_fai, ms_diffuse_fai, ms_specular_fai, ms_depth_fai;
uniform sampler2D light_tex;
uniform sampler2DShadow shadow_map;
uniform vec2 planes; // for the calculation of frag pos in view space

varying vec2 tex_coords;
varying vec3 vpos; // for the calculation of frag pos in view space


/*
=======================================================================================================================================
FragPosVSpace                                                                                                                         =
return frag pos in view space                                                                                                         =
=======================================================================================================================================
*/
vec3 FragPosVSpace()
{
	float _depth = texture2D( ms_depth_fai, tex_coords ).r;

	if( _depth == 1.0 ) discard;

	vec3 _frag_pos_vspace;
	vec3 _vposn = normalize(vpos);
	_frag_pos_vspace.z = -planes.y/(planes.x+_depth);
	_frag_pos_vspace.xy = _vposn.xy/_vposn.z*_frag_pos_vspace.z;
	return _frag_pos_vspace;
}


/*
=======================================================================================================================================
Attenuation                                                                                                                           =
return the attenuation factor fiven the distance from the frag to the light source                                                    =
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
#if defined(_SPOT_LIGHT_) && defined( _SHADOW_ )

float PCF_Off( in vec3 _shadow_uv )
{
	return shadow2D(shadow_map, _shadow_uv ).r;
}


float PCF_Low( in vec3 _shadow_uv )
{
	float _shadow_col = shadow2D(shadow_map, _shadow_uv ).r;
	const float _map_scale = 1.0 / SHADOWMAP_SIZE;

	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3( _map_scale,  _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3( _map_scale, -_map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3( _map_scale,         0.0, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3(-_map_scale,  _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3(-_map_scale, -_map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3(-_map_scale,         0.0, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3(        0.0,  _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3(        0.0, -_map_scale, 0.0)).r;
	_shadow_col *= 1.0/9.0;

	return _shadow_col;
}


float PCF_Medium( in vec3 _shadow_uv )
{
	float _shadow_col = shadow2D(shadow_map, _shadow_uv ).r;
	float _map_scale = 1.0 / SHADOWMAP_SIZE;

	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3( _map_scale,  _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3( _map_scale, -_map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3( _map_scale,         0.0, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3(-_map_scale,  _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3(-_map_scale, -_map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3(-_map_scale,         0.0, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3(        0.0,  _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3(        0.0, -_map_scale, 0.0)).r;

	_map_scale *= 2.0;

	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3( _map_scale,  _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3( _map_scale, -_map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3( _map_scale,         0.0, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3(-_map_scale,  _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3(-_map_scale, -_map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3(-_map_scale,         0.0, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3(        0.0,  _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3(        0.0, -_map_scale, 0.0)).r;

	_shadow_col *= 0.058823529; // aka: _shadow_col /= 17.0;
	return _shadow_col;
}


float PCF_High( in vec3 _shadow_uv )
{
	float _shadow_col = shadow2D(shadow_map, _shadow_uv ).r;
	float _map_scale = 1.0 / SHADOWMAP_SIZE;

	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3( _map_scale,  _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3( _map_scale, -_map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3( _map_scale,  	     0.0, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3(-_map_scale,  _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3(-_map_scale, -_map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3(-_map_scale,  	     0.0, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3(        0.0,  _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3(        0.0, -_map_scale, 0.0)).r;


	float _map_scale_2 = 2.0 * _map_scale;

	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3(_map_scale_2, _map_scale_2, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3(_map_scale, _map_scale_2, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3(0.0, _map_scale_2, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3(-_map_scale, _map_scale_2, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3(-_map_scale_2, _map_scale_2, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3(-_map_scale_2, _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3(-_map_scale_2, 0.0, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3(-_map_scale_2, -_map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3(-_map_scale_2, -_map_scale_2, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3(-_map_scale, -_map_scale_2, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3(0.0, -_map_scale_2, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3(_map_scale, -_map_scale_2, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3(_map_scale_2, -_map_scale_2, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3(_map_scale_2, -_map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3(_map_scale_2, 0.0, 0.0)).r;
	_shadow_col += shadow2D(shadow_map, _shadow_uv.xyz + vec3(_map_scale_2, _map_scale, 0.0)).r;

	_shadow_col /= 25.0;
	return _shadow_col;
}
#endif



/*
=======================================================================================================================================
Phong                                                                                                                                 =
=======================================================================================================================================
*/
vec3 Phong( in vec3 _frag_pos_vspace, out float _frag_light_dist )
{
	// get the lambert term
	vec3 _light_pos_eyespace = gl_LightSource[0].position.xyz;
	vec3 _light_frag_vec = _light_pos_eyespace - _frag_pos_vspace;

	_frag_light_dist = dot( _light_frag_vec, _light_frag_vec ); // instead of using normalize(_frag_light_dist) we brake the operation...
	vec3 _light_dir = _light_frag_vec * inversesqrt(_frag_light_dist); // ...because we want frag_light_dist for the calc of the attenuation

	// read the normal
	//vec3 _normal = texture2D( ms_normal_fai, tex_coords ).rgb;
	vec3 _normal = UnpackNormal( texture2D( ms_normal_fai, tex_coords ).rg );

	// the lambert term
	float _lambert_term = dot( _normal, _light_dir );

	if( _lambert_term < 0.0 ) discard;
	//_lambert_term = max( 0.0, _lambert_term );

	// diffuce lighting
	vec3 _diffuse = texture2D( ms_diffuse_fai, tex_coords ).rgb;
	_diffuse = (_diffuse * gl_LightSource[0].diffuse.rgb);
	vec3 _color = _diffuse * _lambert_term;

	// specular lighting
	vec4 _specular_mix = texture2D( ms_specular_fai, tex_coords );
	vec3 _specular = _specular_mix.xyz;
	float _shininess = _specular_mix.w;

	vec3 _eye_vec = normalize( -_frag_pos_vspace );
	vec3 _h = normalize( _light_dir + _eye_vec );
	float _spec_intensity = pow(max(0.0, dot(_normal, _h)), _shininess);
	_color += _specular * vec3(gl_LightSource[0].specular) * (_spec_intensity * _lambert_term);

	return _color;
}



/*
=======================================================================================================================================
main                                                                                                                                  =
=======================================================================================================================================
*/
void main()
{
	// get frag pos in view space
	vec3 _frag_pos_vspace = FragPosVSpace();

	//===================================================================================================================================
	// POINT LIGHT                                                                                                                      =
	//===================================================================================================================================
	#if defined(_POINT_LIGHT_)
		// The func Phong calculates the frag to light distance (_frag_light_dist) and be cause we need that distance
		// latter for other calculations we export it
		float _frag_light_dist;
		vec3 _color = Phong( _frag_pos_vspace, _frag_light_dist );
		gl_FragData[0] = vec4( _color * Attenuation(_frag_light_dist), 1.0 );

	//===================================================================================================================================
	// SPOT LIGHT                                                                                                                       =
	//===================================================================================================================================
	#elif defined(_SPOT_LIGHT_)
		vec4 _tex_coord2 = gl_TextureMatrix[0] * vec4(_frag_pos_vspace, 1.0);
		vec3 _tex_coords3 = _tex_coord2.xyz / _tex_coord2.w;

		if
		(
			_tex_coord2.w > 0.0 &&
			_tex_coords3.x > 0.0 &&
			_tex_coords3.x < 1.0 &&
			_tex_coords3.y > 0.0 &&
			_tex_coords3.y < 1.0 &&
			_tex_coord2.w < 1.0/gl_LightSource[0].position.w
		)
		{
			#if defined( _SHADOW_ )
				#if defined( _SHADOW_MAPPING_PCF_ )
					float _shadow_color = PCF_Low( _tex_coords3 );
				#else
					float _shadow_color = PCF_Off( _tex_coords3 );
				#endif

				if( _shadow_color == 0.0 ) discard;
			#endif // shadow

			float _frag_light_dist;
			vec3 _color = Phong( _frag_pos_vspace, _frag_light_dist );

			vec3 _texel = texture2DProj( light_tex, _tex_coord2.xyz ).rgb;
			float _att = Attenuation(_frag_light_dist);

			#if defined( _SHADOW_ )
				gl_FragData[0] = vec4(_texel * _color * (_shadow_color * _att), 1.0);
			#else
				gl_FragData[0] = vec4( _color * _texel * _att, 1.0 );
			#endif
			//gl_FragData[0] = vec4( 1.0, 0.0, 1.0, 1.0 );
		}
		else
		{
			discard;
		}
	#endif // spot light

	/*#if defined(_SPOT_LIGHT_)
	gl_FragData[0] = vec4( UnpackNormal(texture2D( ms_normal_fai, tex_coords ).rg), 1.0 );
	//gl_FragData[0] = vec4( texture2D( ms_depth_fai, tex_coords ).rg), 1.0 );
	#endif*/
}
