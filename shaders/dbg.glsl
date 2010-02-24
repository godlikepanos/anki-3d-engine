#pragma anki vertShaderBegins

void main()
{
	gl_Position = ftransform();
	gl_FrontColor = gl_Color;
}

#pragma anki fragShaderBegins

void main()
{
	gl_FragData[0].rgb = gl_Color.rgb;
}
