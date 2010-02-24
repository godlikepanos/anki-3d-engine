#pragma anki vertShaderBegins

varying vec2 tex_coords;

void main(void)
{
	tex_coords = gl_MultiTexCoord0.xy;
	gl_FrontColor = gl_Color;
	gl_Position = ftransform();
}

#pragma anki fragShaderBegins

#pragma anki uniform font_map 0
uniform sampler2D font_map;
varying vec2 tex_coords;

void main()
{
	vec2 tex_col = texture2D( font_map, tex_coords ).ra; // get only one value and the alpha

	if( tex_col.y == 0.0 ) discard;

	gl_FragColor = vec4(tex_col.x) * gl_Color;
}
