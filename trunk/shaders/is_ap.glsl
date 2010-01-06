#pragma anki vert_shader_begins

#pragma anki include "shaders/simple_vert.glsl"

#pragma anki frag_shader_begins

#pragma anki uniform ambient_color 0
uniform vec3 ambient_color;
#pragma anki uniform ms_diffuse_fai 1
uniform sampler2D ms_diffuse_fai;

varying vec2 tex_coords;

void main()
{
	gl_FragData[0].rgb = texture2D( ms_diffuse_fai, tex_coords ).rgb * ambient_color;
}
