varying vec2 txtr_coords;

void main()
{
	txtr_coords = gl_MultiTexCoord0.xy;
	gl_Position = ftransform();
}

