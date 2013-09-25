#pragma anki start vertexShader

#pragma anki include "shaders/SimpleVert.glsl"

#pragma anki start fragmentShader
#pragma anki include shaders/Common.glsl

#if !defined(TILES_X_COUNT) || !defined(TILES_Y_COUNT) || !defined(RENDERER_WIDTH) || !defined(RENDERER_HEIGHT)
#error Forgot to define something
#endif

uniform sampler2D depthMap;

in vec2 vTexCoords;

out uvec2 fColor;

#ifndef GL_ES
layout(pixel_center_integer) in vec4 gl_FragCoord;
#endif

#define W (RENDERER_WIDTH / TILES_X_COUNT)
#define H (RENDERER_HEIGHT / TILES_Y_COUNT)

//==============================================================================
void main()
{
#ifndef GL_ES
	ivec2 coord = ivec2(gl_FragCoord.xy * vec2(W, H));
#else
	ivec2 coord = ivec2((gl_FragCoord.xy - vec2(0.5)) * vec2(W, H));
#endif

	float maxDepth = -10000.0;
	float minDepth = 100000.0;

	for(int i = 0; i < W; i++)
	{
		for(int j = 0; j < H; j++)
		{
			float depth = texelFetch(depthMap, coord + ivec2(i, j), 0).r;

			minDepth = min(depth, minDepth);
			maxDepth = max(depth, maxDepth);
		}
	}

	fColor = uvec2(floatBitsToUint(minDepth), floatBitsToUint(maxDepth));
}


