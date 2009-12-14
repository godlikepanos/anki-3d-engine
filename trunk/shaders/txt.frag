uniform sampler2D font_map;
varying vec2 tex_coords;

void main()
{
	vec2 tex_col = texture2D( font_map, tex_coords ).ra; // get only one value and the alpha

	if( tex_col.y == 0.0 ) discard;

	gl_FragColor = vec4(tex_col.x) * gl_Color;
}
