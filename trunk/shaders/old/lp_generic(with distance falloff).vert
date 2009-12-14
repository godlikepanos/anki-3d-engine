attribute vec3 view_vector;
varying vec3 vpos;
varying vec2 txtr_coord;

void main()
{
	vpos = view_vector;
	txtr_coord = gl_MultiTexCoord0.xy;
  gl_Position = ftransform();
}
