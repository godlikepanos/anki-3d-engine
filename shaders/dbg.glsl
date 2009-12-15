#pragma anki vert_shader_begins

void main()
{
	gl_Position = ftransform();
	gl_FrontColor = gl_Color;
}

#pragma anki frag_shader_begins

void main()
{
	gl_FragData[0].rgb = gl_Color.rgb;
}
