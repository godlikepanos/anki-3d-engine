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

//==============================================================================
void main()
{
	const int FROM_W = RENDERER_WIDTH / TILES_X_COUNT / 2;
	const int FROM_H = RENDERER_HEIGHT / TILES_Y_COUNT / 2;

	float maxDepth = -10.0;
	float minDepth = 10.0;

	for(int i = -FROM_W; i < FROM_W; i++)
	{
		for(int j = -FROM_H; j < FROM_H; j++)
		{
			float depth = textureOffset(depthMap, vTexCoords, ivec2(i, j)).r;

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


