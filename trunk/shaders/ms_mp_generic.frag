/**
Note: The process of calculating the diffuse color for the diffuse MSFAI is divided into two parts. The first happens before the
normal calculation and the other just after it. In the first part we read the texture (or the gl_Color) and we set the _diff_color.
In case of grass we discard. In the second part we calculate a SEM color and we combine it with the _diff_color. We cannot put
the second part before normal calculation because SEM needs the _new_normal. Also we cannot put the first part after normal
calculation because in case of grass we will waste calculations for the normal. For that two reasons we split the diffuse
calculations in two parts
*/

/*
=======================================================================================================================================
VARS                                                                                                                                  =
=======================================================================================================================================
*/
varying vec3 normal;

#if defined( _HAS_DIFFUSE_MAP_ )
	uniform sampler2D diffuse_map;
#endif

#if defined( _HAS_NORMAL_MAP_ )
	uniform sampler2D normal_map;
	varying vec3 tangent_v2f;
	varying float w_v2f;
#endif

#if defined( _HAS_SPECULAR_MAP_ )
	uniform sampler2D specular_map;
#endif

#if defined( _HAS_DIFFUSE_MAP_ ) || defined( _HAS_NORMAL_MAP_ ) || defined( _HAS_SPECULAR_MAP_ ) || defined( _PARALLAX_MAPPING_ )
	varying vec2 tex_coords_v2f;
#endif

#if defined( _PARALLAX_MAPPING_ )
	uniform sampler2D height_map;
	varying vec3 eye;
#endif


#if defined( _ENVIRONMENT_MAPPING_ )
	uniform sampler2D environment_map;
	varying vec3 vert_pos_eye_space_v2f;
#endif


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
	#if defined( _HAS_DIFFUSE_MAP_ )

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
	#if defined( _HAS_NORMAL_MAP_ )
		vec3 _n = normalize( normal );
		vec3 _t = normalize( tangent_v2f );
		vec3 _b = cross(_n, _t) * w_v2f;

		mat3 _tbn_mat = mat3(_t,_b,_n);

		vec3 _n_at_tangentspace = ( texture2D( normal_map, _super_tex_coords ).rgb - 0.5 ) * 2.0;

		vec3 _new_normal = normalize( _tbn_mat * _n_at_tangentspace );
	#else
		vec3 _new_normal = normalize(normal);
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
	#if defined( _HAS_SPECULAR_MAP_ )
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

