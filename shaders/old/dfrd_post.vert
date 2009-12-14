attribute vec3 vpos;
varying vec3 vpos1;

void main()
{
	vpos1 = vpos;
	gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_Position = ftransform();
}

