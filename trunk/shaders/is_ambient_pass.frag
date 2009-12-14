uniform sampler2D ms_diffuse_fai;
uniform vec3 ambient_color;
varying vec2 tex_coords;


void main()
{
	gl_FragData[0].rgb = texture2D( ms_diffuse_fai, tex_coords ).rgb * ambient_color;
}
