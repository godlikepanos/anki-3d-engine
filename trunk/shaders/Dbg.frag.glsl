#pragma anki type frag
#pragma anki include "shaders/Common.glsl"

layout(location = 0) in vec3 inColor;

out vec3 fColor;

void main()
{
	fColor = inColor;
}
