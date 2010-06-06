#pragma anki vertShaderBegins

#pragma anki attribute view_vector 1
attribute vec3 view_vector;
#pragma anki attribute position 0
attribute vec2 position;

varying vec2 texCoords;
varying vec3 vpos;

void main()
{
	vpos = view_vector;
	vec2 vert_pos = position; // the vert coords are {1.0,1.0}, {0.0,1.0}, {0.0,0.0}, {1.0,0.0}
	texCoords = vert_pos;
	vec2 vert_pos_ndc = vert_pos*2.0 - 1.0;
	gl_Position = vec4( vert_pos_ndc, 0.0, 1.0 );
}


#pragma anki fragShaderBegins

#pragma anki include "shaders/pack.glsl"

// uniforms
uniform sampler2D msNormalFai, msDiffuseFai, msSpecularFai, msDepthFai;
uniform vec2 planes; // for the calculation of frag pos in view space
uniform sampler2D lightTex;
uniform sampler2DShadow shadowMap;
uniform vec3 lightPos;
uniform float lightInvRadius;
uniform vec3 lightDiffuseCol;
uniform vec3 lightSpecularCol;
uniform mat4 texProjectionMat;

varying vec2 texCoords;
varying vec3 vpos; // for the calculation of frag pos in view space


/*
=======================================================================================================================================
FragPosVSpace                                                                                                                         =
return frag pos in view space                                                                                                         =
=======================================================================================================================================
*/
vec3 FragPosVSpace()
{
	float _depth = texture2D( msDepthFai, texCoords ).r;

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
	return clamp(1.0 - lightInvRadius * sqrt(_frag_light_dist), 0.0, 1.0);
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
	return shadow2D(shadowMap, _shadow_uv ).r;
}


float PCF_Low( in vec3 _shadow_uv )
{
	float _shadow_col = shadow2D(shadowMap, _shadow_uv ).r;
	const float _map_scale = 1.0 / SHADOWMAP_SIZE;

	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3( _map_scale,  _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3( _map_scale, -_map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3( _map_scale,         0.0, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3(-_map_scale,  _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3(-_map_scale, -_map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3(-_map_scale,         0.0, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3(        0.0,  _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3(        0.0, -_map_scale, 0.0)).r;
	_shadow_col *= 1.0/9.0;

	return _shadow_col;
}


float PCF_Medium( in vec3 _shadow_uv )
{
	float _shadow_col = shadow2D(shadowMap, _shadow_uv ).r;
	float _map_scale = 1.0 / SHADOWMAP_SIZE;

	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3( _map_scale,  _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3( _map_scale, -_map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3( _map_scale,         0.0, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3(-_map_scale,  _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3(-_map_scale, -_map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3(-_map_scale,         0.0, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3(        0.0,  _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3(        0.0, -_map_scale, 0.0)).r;

	_map_scale *= 2.0;

	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3( _map_scale,  _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3( _map_scale, -_map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3( _map_scale,         0.0, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3(-_map_scale,  _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3(-_map_scale, -_map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3(-_map_scale,         0.0, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3(        0.0,  _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3(        0.0, -_map_scale, 0.0)).r;

	_shadow_col *= 0.058823529; // aka: _shadow_col /= 17.0;
	return _shadow_col;
}


float PCF_High( in vec3 _shadow_uv )
{
	float _shadow_col = shadow2D(shadowMap, _shadow_uv ).r;
	float _map_scale = 1.0 / SHADOWMAP_SIZE;

	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3( _map_scale,  _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3( _map_scale, -_map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3( _map_scale,  	     0.0, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3(-_map_scale,  _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3(-_map_scale, -_map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3(-_map_scale,  	     0.0, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3(        0.0,  _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3(        0.0, -_map_scale, 0.0)).r;


	float _map_scale_2 = 2.0 * _map_scale;

	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3(_map_scale_2, _map_scale_2, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3(_map_scale, _map_scale_2, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3(0.0, _map_scale_2, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3(-_map_scale, _map_scale_2, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3(-_map_scale_2, _map_scale_2, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3(-_map_scale_2, _map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3(-_map_scale_2, 0.0, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3(-_map_scale_2, -_map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3(-_map_scale_2, -_map_scale_2, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3(-_map_scale, -_map_scale_2, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3(0.0, -_map_scale_2, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3(_map_scale, -_map_scale_2, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3(_map_scale_2, -_map_scale_2, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3(_map_scale_2, -_map_scale, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3(_map_scale_2, 0.0, 0.0)).r;
	_shadow_col += shadow2D(shadowMap, _shadow_uv.xyz + vec3(_map_scale_2, _map_scale, 0.0)).r;

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
	vec3 _lightPos_eyespace = lightPos;
	vec3 _light_frag_vec = _lightPos_eyespace - _frag_pos_vspace;

	_frag_light_dist = dot( _light_frag_vec, _light_frag_vec ); // instead of using normalize(_frag_light_dist) we brake the operation...
	vec3 _light_dir = _light_frag_vec * inversesqrt(_frag_light_dist); // ...because we want frag_light_dist for the calc of the attenuation

	// read the normal
	//vec3 _normal = texture2D( msNormalFai, texCoords ).rgb;
	vec3 _normal = UnpackNormal( texture2D( msNormalFai, texCoords ).rg );

	// the lambert term
	float _lambert_term = dot( _normal, _light_dir );

	if( _lambert_term < 0.0 ) discard;
	//_lambert_term = max( 0.0, _lambert_term );

	// diffuce lighting
	vec3 _diffuse = texture2D( msDiffuseFai, texCoords ).rgb;
	_diffuse = (_diffuse * lightDiffuseCol);
	vec3 _color = _diffuse * _lambert_term;

	// specular lighting
	vec4 _specular_mix = texture2D( msSpecularFai, texCoords );
	vec3 _specular = _specular_mix.xyz;
	float _shininess = _specular_mix.w;

	vec3 _eye_vec = normalize( -_frag_pos_vspace );
	vec3 _h = normalize( _light_dir + _eye_vec );
	float _spec_intensity = pow(max(0.0, dot(_normal, _h)), _shininess);
	_color += _specular * lightSpecularCol * (_spec_intensity * _lambert_term);

	return _color;
}

uniform sampler2D tex;

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
		vec4 _tex_coord2 = texProjectionMat * vec4(_frag_pos_vspace, 1.0);
		vec3 _texCoords3 = _tex_coord2.xyz / _tex_coord2.w;

		if
		(
			_tex_coord2.w > 0.0 &&
			_texCoords3.x > 0.0 &&
			_texCoords3.x < 1.0 &&
			_texCoords3.y > 0.0 &&
			_texCoords3.y < 1.0 &&
			_tex_coord2.w < 1.0/lightInvRadius
		)
		{
			#if defined( _SHADOW_ )
				#if defined( _SHADOW_MAPPING_PCF_ )
					float _shadow_color = PCF_Low( _texCoords3 );
					//float _shadow_color = MedianFilterPCF( shadowMap, _texCoords3 );
				#else
					float _shadow_color = PCF_Off( _texCoords3 );
				#endif

				if( _shadow_color == 0.0 ) discard;
			#endif // shadow

			float _frag_light_dist;
			vec3 _color = Phong( _frag_pos_vspace, _frag_light_dist );

			vec3 _texel = texture2DProj( lightTex, _tex_coord2.xyz ).rgb;
			float _att = Attenuation(_frag_light_dist);

			#if defined( _SHADOW_ )
				gl_FragData[0] = vec4(_texel * _color * (_shadow_color * _att), 1.0);
			#else
				gl_FragData[0] = vec4( _color * _texel * _att, 1.0 );
			#endif
		}
		else
		{
			discard;
		}
	#endif // spot light

	/*#if defined(_SPOT_LIGHT_)
	gl_FragData[0] = vec4( UnpackNormal(texture2D( msNormalFai, texCoords ).rg), 1.0 );
	//gl_FragData[0] = vec4( texture2D( msDepthFai, texCoords ).rg), 1.0 );
	#endif*/
}
