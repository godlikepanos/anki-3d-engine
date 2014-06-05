// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// convert to linear depth

float linearizeDepth(in float depth, in float zNear, in float zFar)
{
	return (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

float readFromTextureAndLinearizeDepth(in sampler2D depthMap, in vec2 texCoord, 
	in float zNear, in float zFar)
{
	return linearizeDepth(texture(depthMap, texCoord).r, zNear, zFar);
}
