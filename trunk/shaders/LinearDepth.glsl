// convert to linear depth

float linearizeDepth(in float depth, in float zNear, in float zFar)
{
	return (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

float readFromTexAndLinearizeDepth(in sampler2D depthMap, in vec2 texCoord, in float zNear, in float zFar)
{
	return linearizeDepth(texture2D(depthMap, texCoord).r, zNear, zFar);
}
