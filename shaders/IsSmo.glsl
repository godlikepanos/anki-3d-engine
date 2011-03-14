/// @file
///
/// Ilunimation stage stencil masking optimizations shader program

#pragma anki vertShaderBegins

layout(location = 0) in vec3 position;

uniform mat4 modelViewProjectionMat;

void main()
{
	gl_Position = modelViewProjectionMat * vec4(position, 1.0);
}

#pragma anki fragShaderBegins

void main()
{
}
