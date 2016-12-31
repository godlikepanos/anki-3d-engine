// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Common.glsl"

layout(ANKI_TEX_BINDING(0, 0)) uniform sampler2D u_depthRt;

layout(location = 0) in vec2 in_uv;

#define AVG

void main()
{
	vec4 depths = textureGather(u_depthRt, in_uv, 0);

#if defined(MIN)
	vec2 mind2 = min(depths.xy, depths.zw);
	gl_FragDepth = min(mind2.x, mind2.y);
#elif defined(MAX)
	vec2 max2 = max(depths.xy, depths.zw);
	gl_FragDepth = max(max2.x, max2.y);
#elif defined(AVG)
	gl_FragDepth = dot(depths, vec4(1.0 / 4.0));
#else
#error See file
#endif
}
