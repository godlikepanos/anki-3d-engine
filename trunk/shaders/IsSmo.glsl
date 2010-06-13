#pragma anki vertShaderBegins

#pragma anki attribute position 0
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
