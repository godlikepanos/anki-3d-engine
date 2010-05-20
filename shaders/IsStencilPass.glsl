#pragma anki vertShaderBegins

attribute vec3 position;
uniform mat4 modelViewProjectionMat;

void main()
{
	gl_Position = modelViewProjectionMat * vec4( position, 1.0 );
}

#pragma anki fragShaderBegins

void main()
{
}
