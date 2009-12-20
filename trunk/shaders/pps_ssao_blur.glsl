#pragma anki vert_shader_begins

#pragma anki include "shaders/simple_vert.glsl"

#pragma anki frag_shader_begins

#pragma anki include "shaders/median_filter.glsl"

varying vec2 tex_coords;

#pragma anki uniform tex 0
uniform sampler2D tex;

void main()
{
	gl_FragColor.a = MedianFilterAlpha( tex, tex_coords );
}
