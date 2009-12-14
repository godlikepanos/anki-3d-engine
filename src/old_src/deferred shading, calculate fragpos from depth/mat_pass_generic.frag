varying vec3 normal;

#if defined( _HAS_DIFFUSE_MAP_ )
uniform sampler2D diffuse_map;
#endif

#if defined( _HAS_NORMAL_MAP_ )
uniform sampler2D normal_map;
varying vec3 tangent_v;
#endif

#if defined( _HAS_DIFFUSE_MAP_ ) || defined( _HAS_NORMAL_MAP_ )
varying vec2 txtr_coords;
#endif



/*
=======================================================================================================================================
main                                                                                                                                  =
=======================================================================================================================================
*/
void main()
{
		// DIFFUSE COLOR
#if defined( _HAS_DIFFUSE_MAP_ )
	vec4 _diffuse = texture2D( diffuse_map, txtr_coords );

	#if defined( _GRASS_LIKE_ )
		if( _diffuse.a == 0.0 ) discard;
	#endif

	gl_FragData[1] = _diffuse;
#else
	gl_FragData[1] = gl_Color;//gl_FrontMaterial.diffuse;
#endif


	// NORMAL
#if defined( _HAS_NORMAL_MAP_ )
	vec3 _n = normalize( normal );
	vec3 _t = normalize( tangent_v );
	vec3 _b = cross(_n, _t);

	mat3 _tbn_mat = mat3(_t.x, _b.x, _n.x, _t.y, _b.y, _n.y, _t.z, _b.z, _n.z);

	vec3 _ntex = ( texture2D( normal_map, txtr_coords ).rgb - 0.5 ) * 2.0;

	vec3 _ncol = normalize( _ntex * _tbn_mat ) * 0.5 + 0.5;
	gl_FragData[0] = vec4(_ncol, 1.0);
#else
	vec3 _ncol = normalize(normal) * 0.5 + 0.5; // normalize and convert from [-1,1] to color space, aka [0,1]
	gl_FragData[0] = vec4(_ncol, 1.0);
#endif

	// SPECULAR COLOR
	gl_FragData[2] = vec4(gl_FrontMaterial.specular.rgb, gl_FrontMaterial.shininess/128.0);


	/*vec3 _normal = normalize(normal);
	_normal = Normal2Color(_normal);
	vec3 _nx = float_to_color(_normal.x);
	vec3 _ny = float_to_color(_normal.y);
	vec3 _nz = float_to_color(_normal.z);

	gl_FragData[0] = vec4(_nx, _nz.x);

	gl_FragData[1] = gl_Color;

	gl_FragData[2] = vec4(gl_FrontMaterial.specular.rgb, _nz.z);

	gl_FragData[3] = vec4( _ny, _nz.y);*/

}

