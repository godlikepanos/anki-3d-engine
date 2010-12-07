///
#pragma anki vertShaderBegins

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

uniform mat4 modelViewProjectionMat;

out vec3 vColor;

void main()
{
	vColor = color;
	gl_Position = modelViewProjectionMat * vec4(position, 1.0);
}

#pragma anki fragShaderBegins

in vec3 vColor;

out vec3 fColor;

void main()
{
	fColor = vColor;
}
