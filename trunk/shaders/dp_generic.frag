uniform sampler2D diffuse_map;
varying vec2 txtr_coords;



void main()
{
	#if defined( _GRASS_LIKE_ )
		vec4 _diff = texture2D( diffuse_map, txtr_coords );
		if( _diff.a == 0.0 ) discard;
	#endif
}
