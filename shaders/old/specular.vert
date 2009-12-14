varying vec3 normal, vert_pos;

void main()
{
	normal = gl_NormalMatrix * gl_Normal;
  vert_pos = (gl_ModelViewMatrix * gl_Vertex).xyz; // vert pos in world space
  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_Position = ftransform();
}
