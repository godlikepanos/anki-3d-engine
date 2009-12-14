varying vec2 tex_coords;
uniform sampler2D diff_map;

void main()
{
	gl_FragColor = texture2D( diff_map, tex_coords );
}
