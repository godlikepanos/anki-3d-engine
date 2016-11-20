// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Common.glsl"

layout(ANKI_TEX_BINDING(0, 0)) uniform sampler2D u_depthRt;

layout(location = 0) in vec2 in_uv;

void main()
{
	vec4 depths = textureGather(u_depthRt, in_uv, 0);

	vec2 mind2 = min(depths.xy, depths.zw);
	float mind = min(mind2.x, mind2.y);

	gl_FragDepth = mind;
}
