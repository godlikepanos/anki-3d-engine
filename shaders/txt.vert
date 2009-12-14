varying vec2 tex_coords;

void main(void)
{
	tex_coords = gl_MultiTexCoord0.xy;
	gl_FrontColor = gl_Color;
	gl_Position = ftransform();
}


