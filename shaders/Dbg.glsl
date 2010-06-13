#pragma anki vertShaderBegins

#pragma anki attribute position 0
attribute vec3 position;
#pragma anki attribute color 1
attribute vec3 color;

uniform mat4 modelViewProjectionMat;

varying vec3 colorV2f;

void main()
{
	colorV2f = color;
	gl_Position = modelViewProjectionMat * vec4( position, 1.0 );
}

#pragma anki fragShaderBegins

varying vec3 colorV2f;

void main()
{
	gl_FragData[0].rgb = colorV2f;
}
