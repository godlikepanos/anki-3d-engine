// Application to vertex shader
varying vec3 normal_v;
varying vec3 vert_pos_modelview_v;

void main()
{
	vert_pos_modelview_v = (gl_ModelViewMatrix * gl_Vertex).xyz;

	normal_v  = gl_NormalMatrix * gl_Normal;

	gl_FrontColor = gl_Color;
	gl_Position = ftransform();
}
