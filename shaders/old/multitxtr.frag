uniform sampler2D t0, t1;


void main()
{
	gl_FragColor = texture2D( t0, gl_TexCoord[0].st) * texture2D( t1, gl_TexCoord[0].st);
}
