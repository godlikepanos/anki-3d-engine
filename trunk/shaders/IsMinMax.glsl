#pragma anki start vertexShader

#pragma anki include "shaders/SimpleVert.glsl"

#pragma anki start fragmentShader

#if !defined(TILES_X_COUNT) || !defined(TILES_Y_COUNT) || !defined(RENDERER_WIDTH) || !defined(RENDERER_HEIGHT)
#	error "See file"
#endif

uniform vec2 nearFar;
#define near nearFar.x
#define far nearFar.y

uniform sampler2D depthMap;

in vec2 vTexCoords;

out uvec2 fColor;

layout(pixel_center_integer) in vec4 gl_FragCoord;

#define W (RENDERER_WIDTH / TILES_X_COUNT)
#define H (RENDERER_HEIGHT / TILES_Y_COUNT)

//==============================================================================
void main()
{
	const ivec2 coord = ivec2(gl_FragCoord.xy * vec2(W, H));

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


