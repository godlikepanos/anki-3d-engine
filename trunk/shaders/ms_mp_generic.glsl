#if defined( _DIFFUSE_MAPPING_ ) || defined( _NORMAL_MAPPING_ ) || defined( _SPECULAR_MAPPING_ )
	#define NEEDS_TEX_MAPPING 1
#else
	#define NEEDS_TEX_MAPPING 0
#endif


#if defined( _NORMAL_MAPPING_ ) || defined( _PARALLAX_MAPPING_ )
	#define NEEDS_TANGENT 1
#else
	#define NEEDS_TANGENT 0
#endif


#pragma anki vert_shader_begins

/**
 * This a generic shader to fill the deferred shading buffers. You can always build your own if you dont need to write in all the
 * buffers.
 */
#pragma anki include "shaders/hw_skinning.glsl"

// attributes
attribute vec3 position;
attribute vec3 normal;
attribute vec2 tex_coords;
attribute vec4 tangent;

// uniforms
uniform mat4 MVP_mat;
uniform mat4 MV_mat;
uniform mat3 N_mat;

// varyings
varying vec3 normal_v2f;
varying vec2 tex_coords_v2f;
varying vec3 tangent_v2f;
varying float w_v2f;
varying vec3 eye_v2f; ///< In tangent space
varying vec3 vert_pos_eye_space_v2f;



//=====================================================================================================================================
// main                                                                                                                               =
//=====================================================================================================================================
void main()
{
	// calculate the vert pos, normal and tangent

	// if we have hardware skinning then:
	#if defined( _HARDWARE_SKINNING_ )
		mat3 _rot;
		vec3 _tsl;

		HWSkinning( _rot, _tsl );

		normal_v2f = gl_NormalMatrix * ( _rot * normal );

		#if NEEDS_TANGENT
			tangent_v2f = gl_NormalMatrix * ( _rot * vec3(tangent) );
		#endif

		vec3 pos_lspace = ( _rot * position) + _tsl;
		gl_Position =  gl_ModelViewProjectionMatrix * vec4(pos_lspace, 1.0);

	// if DONT have hardware skinning
	#else
		normal_v2f = gl_NormalMatrix * normal;

		#if NEEDS_TANGENT
			tangent_v2f = gl_NormalMatrix * vec3(tangent);
		#endif

		gl_Position = gl_ModelViewProjectionMatrix * vec4(position, 1.0);
	#endif


	// calculate the rest

	#if NEEDS_TEX_MAPPING
		tex_coords_v2f = tex_coords;
	#endif


	#if NEEDS_TANGENT
		w_v2f = tangent.w;
	#endif


	#if defined( _ENVIRONMENT_MAPPING_ )
		vert_pos_eye_space_v2f = vec3( gl_ModelViewMatrix * vec4(position, 1.0) );
	#endif


	#if defined( _PARALLAX_MAPPING_ )
		/*vec3 t = gl_NormalMatrix * tangent.xyz;
		vec3 n = normal;
		vec3 b = cross( n, t )  tangent.w;

		vec3 eye_pos = gl_Position.xyz - gl_ModelViewMatrixInverse[3].xyz;
		eye_pos = eye_pos - ( gl_ModelViewMatrixInverse * gl_Vertex ).xyz;

		mat3 tbn_mat = mat3( t, b, n );
		eye = tbn_mat * eye_pos;
		//eye.y = -eye.y;
		//eye.x = -eye.x;*/
	#endif
}


#pragma anki frag_shader_begins

/**
 * Note: The process of calculating the diffuse color for the diffuse MSFAI is divided into two parts. The first happens before the
 * normal calculation and the other just after it. In the first part we read the texture (or the gl_Color) and we set the _diff_color.
 * In case of grass we discard. In the second part we calculate a SEM color and we combine it with the _diff_color. We cannot put
 * the second part before normal calculation because SEM needs the _new_normal. Also we cannot put the first part after normal
 * calculation because in case of grass we will waste calculations for the normal. For that two reasons we split the diffuse
 * calculations in two parts
 */

#pragma anki include "shaders/pack.glsl"

/*
=======================================================================================================================================
VARS                                                                                                                                  =
=======================================================================================================================================
*/

uniform sampler2D diffuse_map;
uniform sampler2D normal_map;
uniform sampler2D specular_map;
uniform sampler2D height_map;
uniform sampler2D environment_map;

varying vec3 normal_v2f;
varying vec3 tangent_v2f;
varying float w_v2f;
varying vec2 tex_coords_v2f;
varying vec3 eye;
varying vec3 vert_pos_eye_space_v2f;



/*
=======================================================================================================================================
main                                                                                                                                  =
=======================================================================================================================================
*/
void main()
{
	//===================================================================================================================================
	// Paralax Mapping Calculations                                                                                                     =
	// The code below reads the height map, makes some calculations and returns a new tex_coords                                        =
	//===================================================================================================================================
	#if defined( _PARALLAX_MAPPING_ )
		const float _scale = 0.04;
		const float _bias = scale * 0.4;

		vec3 _norm_eye = normalize( eye );

		float _h = texture2D( height_map, tex_coords_v2f ).r;
		float _height = _scale * _h - _bias;

		vec2 _super_tex_coords_v2f = _height * _norm_eye.xy + tex_coords_v2f;
	#else
		#define _super_tex_coords tex_coords_v2f
	#endif


	//===================================================================================================================================
	// Diffuse Calculations (Part I)                                                                                                    =
	// Get the color from the diffuse map and discard if grass                                                                          =
	//===================================================================================================================================
	vec3 _diff_color;
	#if defined( _DIFFUSE_MAPPING_ )

		#if defined( _GRASS_LIKE_ )
			vec4 _diff_color4 = texture2D( diffuse_map, _super_tex_coords );
			if( _diff_color4.a == 0.0 ) discard;
			_diff_color = _diff_color4.rgb;
		#else
			_diff_color = texture2D( diffuse_map, _super_tex_coords ).rgb;
		#endif

		_diff_color *= gl_FrontMaterial.diffuse.rgb;
	#else
		_diff_color = gl_FrontMaterial.diffuse.rgb;
	#endif


	//===================================================================================================================================
	// Normal Calculations                                                                                                              =
	// Either use a normap map and make some calculations or use the vertex normal                                                      =
	//===================================================================================================================================
	#if defined( _NORMAL_MAPPING_ )
		vec3 _n = normalize( normal_v2f );
		vec3 _t = normalize( tangent_v2f );
		vec3 _b = cross(_n, _t) * w_v2f;

		mat3 _tbn_mat = mat3(_t,_b,_n);

		vec3 _n_at_tangentspace = ( texture2D( normal_map, _super_tex_coords ).rgb - 0.5 ) * 2.0;

		vec3 _new_normal = normalize( _tbn_mat * _n_at_tangentspace );
	#else
		vec3 _new_normal = normalize(normal_v2f);
	#endif


	//===================================================================================================================================
	// Diffuse Calculations (Part II)                                                                                                   =
	// If SEM is enabled make some calculations and combine colors of SEM and the _diff_color                                           =
	//===================================================================================================================================

	// if SEM enabled make some aditional calculations using the vert_pos_eye_space_v2f, environment_map and the _new_normal
	#if defined( _ENVIRONMENT_MAPPING_ )
		vec3 _u = normalize( vert_pos_eye_space_v2f );
		vec3 _r = reflect( _u, _new_normal ); // In case of normal mapping I could play with vertex's normal but this gives better...
		                                      // ...results and its allready computed
		_r.z += 1.0;
		float _m = 2.0 * length(_r);
		vec2 _sem_tex_coords = _r.xy/_m + 0.5;

		vec3 _sem_col = texture2D( environment_map, _sem_tex_coords ).rgb;
		_diff_color = _diff_color + _sem_col; // blend existing color with the SEM texture map
	#endif


	//===================================================================================================================================
	// Specular Calculations                                                                                                            =
	//===================================================================================================================================

	// has specular map
	#if defined( _SPECULAR_MAPPING_ )
		vec4 _specular = vec4(texture2D( specular_map, _super_tex_coords ).rgb * gl_FrontMaterial.specular.rgb, gl_FrontMaterial.shininess);
	// no specular map
	#else
		vec4 _specular = vec4(gl_FrontMaterial.specular.rgb, gl_FrontMaterial.shininess);
	#endif


	//===================================================================================================================================
	// Final Stage. Write all data                                                                                                      =
	//===================================================================================================================================
	//gl_FragData[0].rgb = _new_normal;
	gl_FragData[0].rg = PackNormal( _new_normal );
	gl_FragData[1].rgb = _diff_color;
	gl_FragData[2] = _specular;

	/*#if defined( _HARDWARE_SKINNING_ )
		gl_FragData[1] = gl_Color;
	#endif*/
}



