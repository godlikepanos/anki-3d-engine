varying vec2 txtr_coords;

void main()
{
	#if defined( _GRASS_LIKE_ )
		txtr_coords = gl_MultiTexCoord0.xy;
	#endif

	#if defined( _HW_SKINNING_ )
		mat3 _rot;
		vec3 _tsl;

		HWSkinning( _rot, _tsl );
		
		vec3 pos_lspace = ( _rot * gl_Vertex.xyz) + _tsl;
		gl_Position =  gl_ModelViewProjectionMatrix * vec4(pos_lspace, 1.0);
	#else
	  gl_Position = ftransform();
	#endif
}
