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

	float mind = min(depths.x, depths.y);
	mind = min(mind, depths.z);
	mind = min(mind, depths.w);

	gl_FragDepth = mind;
}
