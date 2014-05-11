#pragma anki type vert
#pragma anki include "shaders/Common.glsl"

/// the vert coords are NDC
layout(location = POSITION_LOCATION) in vec2 inPosition;

layout(location = 0) out vec2 outTexCoord;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	outTexCoord = (inPosition * 0.5) + 0.5;
	gl_Position = vec4(inPosition, 0.0, 1.0);
}
