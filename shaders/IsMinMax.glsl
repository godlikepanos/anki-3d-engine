#pragma anki start vertexShader

#pragma anki include "shaders/SimpleVert.glsl"

#pragma anki start fragmentShader

#pragma anki include "shaders/LinearDepth.glsl"

#if !defined(TILES_X_COUNT) || !defined(TILES_Y_COUNT) || !defined(RENDERER_WIDTH) || !defined(RENDERER_HEIGHT)
#	error "See file"
#endif

uniform vec2 nearFar;
uniform sampler2D depthMap;

in vec2 vTexCoords;

out vec2 fColor;

//==============================================================================
void main()
{
#if 1
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

	fColor = vec2(linearizeDepth(minDepth, nearFar.x, nearFar.y),
		linearizeDepth(maxDepth, nearFar.x, nearFar.y));
#else
	fColor = vec2(float(gl_FragCoord.x) / TILES_X_COUNT, 
		float(gl_FragCoord.y) / TILES_Y_COUNT);
#endif
}


