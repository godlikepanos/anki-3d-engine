// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma anki type frag
#pragma anki include "shaders/Common.glsl"

uniform sampler2D u_depthMap;

layout(location = 0) out vec2 out_color;

layout(pixel_center_integer) in vec4 gl_FragCoord;

#define W (RENDERER_WIDTH / TILES_X_COUNT)
#define H (RENDERER_HEIGHT / TILES_Y_COUNT)

//==============================================================================
void main()
{
	ivec2 coord = ivec2(gl_FragCoord.xy * vec2(W, H));

	float maxDepth = -10000.0;
	float minDepth = 100000.0;

	for(int x = 0; x < W; x++)
	{
		for(int y = 0; y < H; y++)
		{
			float depth = texelFetch(u_depthMap, coord + ivec2(x, y), 0).r;

			minDepth = min(depth, minDepth);
			maxDepth = max(depth, maxDepth);
		}
	}

	out_color = vec2(minDepth, maxDepth);
}


