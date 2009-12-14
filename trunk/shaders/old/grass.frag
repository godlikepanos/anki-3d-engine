uniform sampler2D t0;

void main()
{
	vec4 tex_col = texture2D( t0, gl_TexCoord[0].st );
	
	if( tex_col.a == 0.0 ) discard;
	
	gl_FragColor = tex_col;
}
