varying vec2 tex_coords;
varying vec3 normal;

void main()
{
	tex_coords = gl_MultiTexCoord0.xy;
	normal = gl_NormalMatrix * gl_Normal;
	gl_Position = ftransform();
}
