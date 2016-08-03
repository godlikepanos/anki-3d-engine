// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// The final pass

#include "shaders/Common.glsl"
#include "shaders/Pack.glsl"

layout(ANKI_TEX_BINDING(0, 0)) uniform sampler2D u_tex;

layout(location = 0) in highp vec2 in_texCoord;
layout(location = 0) out vec3 out_color;

void main()
{
	vec3 col = textureLod(u_tex, in_texCoord, 0.0).rgb;

	/*vec2 depth = textureRt(u_tex, in_texCoord).rg;
	float zNear = 0.2;
	float zFar = 200.0;
	depth = (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
	vec3 col = vec3(depth.rg, 0.0);*/

	out_color = col;
}
