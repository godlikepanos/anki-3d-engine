// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// The final pass

#pragma anki type frag

#pragma anki include "shaders/Common.glsl"
#pragma anki include "shaders/Pack.glsl"

layout(binding = 0) uniform sampler2D uRasterImage;

layout(location = 0) in highp vec2 inTexCoords;
layout(location = 0) out vec3 outFragColor;

void main()
{
	vec3 col = textureRt(uRasterImage, inTexCoords).rgb;
	
	/*vec2 depth = textureRt(uRasterImage, inTexCoords).rg;
	float zNear = 0.2;
	float zFar = 200.0;
	depth = (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
	vec3 col = vec3(depth.rg, 0.0);*/

	outFragColor = col;
}
