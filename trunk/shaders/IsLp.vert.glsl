#pragma anki type vert

#if !TILES_X_COUNT || !TILES_Y_COUNT
#	error See file
#endif

#pragma anki include "shaders/IsCommon.glsl"

layout(location = 0) in vec2 inPosition;

layout(location = 0) out vec2 outTexCoord;
layout(location = 1) flat out int outInstanceId;
layout(location = 2) out vec2 outProjectionParams;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	float instIdF = float(gl_InstanceID);

	vec2 ij = vec2(
		mod(instIdF, float(TILES_X_COUNT)), 
		floor(instIdF / float(TILES_X_COUNT)));

	outInstanceId = int(gl_InstanceID);

	const vec2 SIZES = 
		vec2(1.0 / float(TILES_X_COUNT), 1.0 / float(TILES_Y_COUNT));

	outTexCoord = (inPosition + ij) * SIZES;
	vec2 vertPosNdc = outTexCoord * 2.0 - 1.0;
	gl_Position = vec4(vertPosNdc, 0.0, 1.0);

	outProjectionParams = uProjectionParams.xy * vertPosNdc;
}


