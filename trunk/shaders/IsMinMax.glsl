#pragma anki start vertexShader

#pragma anki include "shaders/SimpleVert.glsl"

#pragma anki start fragmentShader

#pragma anki include "shaders/LinearDepth.glsl"

#if !defined(TILES_X_COUNT) || !defined(TILES_Y_COUNT) || !defined(RENDERER_WIDTH) || !defined(RENDERER_HEIGHT)
#	error "See file"
#endif

uniform vec2 nearFar;
#define near nearFar.x
#define far nearFar.y

uniform sampler2D depthMap;

in vec2 vTexCoords;

out vec2 fColor;

layout(pixel_center_integer) in vec4 gl_FragCoord;

//==============================================================================
void main()
{
	const int W = RENDERER_WIDTH / TILES_X_COUNT;
	const int H = RENDERER_HEIGHT / TILES_Y_COUNT;

	float maxDepth = -10000.0;
	float minDepth = 100000.0;

	ivec2 coord = ivec2(gl_FragCoord.xy * vec2(W, H));

	for(int i = 0; i < W; i++)
	{
		for(int j = 0; j < H; j++)
		{
			float depth = texelFetch(depthMap, coord + ivec2(i, j), 0).r;

			if(depth < minDepth)
			{
				minDepth = depth;
			}
			if(depth > maxDepth)
			{
				maxDepth = depth;
			}
		}
	}

#if 0
	float linearMin = linearizeDepth(minDepth, near, far);
	float linearMax = linearizeDepth(maxDepth, near, far);
	fColor = vec2(linearMin,
		linearMax);
#else
	fColor = vec2(minDepth, maxDepth);
#endif
}


