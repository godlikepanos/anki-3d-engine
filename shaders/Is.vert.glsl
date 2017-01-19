// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Common.glsl"

layout(location = 0) out vec2 out_uv;
layout(location = 1) out vec2 out_clusterIJ;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	const vec2 POSITIONS[3] = vec2[](vec2(-1.0, -1.0), vec2(3.0, -1.0), vec2(-1.0, 3.0));

	vec2 pos = POSITIONS[gl_VertexID];
	out_uv = NDC_TO_UV(pos);
	ANKI_WRITE_POSITION(vec4(pos, 0.0, 1.0));

	out_clusterIJ = vec2(CLUSTER_COUNT_X, CLUSTER_COUNT_Y) * out_uv;
}
