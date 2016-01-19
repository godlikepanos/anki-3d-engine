// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Convert to linear depth
float linearizeDepth(in float depth, in float zNear, in float zFar)
{
	return (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

// This is the optimal linearizeDepth where a=(f+n)/2n and b=(n-f)/2n
float linearizeDepthOptimal(in float depth, in float a, in float b)
{
	return 1.0 / (a + depth * b);
}
