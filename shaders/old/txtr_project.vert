//varying vec3 normal, vert_pos;
//varying vec4 txtr_coord;
//
//void main()
//{
//	normal = gl_NormalMatrix * gl_Normal;
//	vert_pos = vec3(gl_ModelViewMatrix * gl_Vertex); // vert pos in world space
//	txtr_coord = gl_TextureMatrix[0] * vec4(vert_pos, 0.0);
//
//  //gl_TexCoord[0] = gl_MultiTexCoord0;
//  gl_Position = ftransform();
//}


varying vec3 normal, vert_light_vec, eye_vec;
varying vec4 txtr_coord;

void main()
{
	/// standard stuff
	//gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_Position = ftransform();

	/// varying vars
	normal = gl_NormalMatrix * gl_Normal;

	vec4 _vert_pos_eye_space = gl_ModelViewMatrix * gl_Vertex;
	vert_light_vec = gl_LightSource[0].position.xyz - _vert_pos_eye_space.xyz;
	eye_vec = -_vert_pos_eye_space.xyz; // the actual eye_vec is normalized but there is no need to normalize in vert shader

	txtr_coord = gl_TextureMatrix[0] * _vert_pos_eye_space;
}

