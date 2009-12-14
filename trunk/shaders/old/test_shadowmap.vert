varying vec4 txtr_coord;

void main()
{
	vec4 realPos = gl_ModelViewMatrix * gl_Vertex;
  
	txtr_coord = gl_TextureMatrix[0] * realPos;
	gl_FrontColor = gl_Color;

	gl_Position = ftransform();
}


