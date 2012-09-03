#if !TILES_X_COUNT || !TILES_Y_COUNT
#	error "See file"
#endif

layout(location = 0) in vec2 position;

out vec2 vTexCoords;
flat out uint vInstanceId;

void main()
{
	vec2 ij = vec2(
		float(gl_InstanceID % TILES_X_COUNT), 
		float(gl_InstanceID / TILES_X_COUNT));

	vInstanceId = gl_InstanceID;

	const vec2 SIZES = vec2(1.0 / TILES_X_COUNT, 1.0 / TILES_Y_COUNT);

	vTexCoords = (position + ij) * SIZES;
	vec2 vertPosNdc = vTexCoords * 2.0 - 1.0;
	gl_Position = vec4(vertPosNdc, 0.0, 1.0);
}
