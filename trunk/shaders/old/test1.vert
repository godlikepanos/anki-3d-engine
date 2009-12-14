varying vec3 txtr_coord;

void main()
{
	vec4 _vert_pos_eye_space = gl_ModelViewMatrix * gl_Vertex;
	txtr_coord = vec3(gl_TextureMatrix[0] * _vert_pos_eye_space);

	gl_FrontColor = gl_Color;

  gl_Position = ftransform();
}

