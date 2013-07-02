// This compute shader rejects lights

#pragma anki start computeShader

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

#define TILE_W (DEPTHMAP_WIDTH / TILES_X_COUNT)
#define TILE_H (DEPTHMAP_HEIGHT / TILES_Y_COUNT)

#define tileX gl_WorkGroupID.x
#define tileY gl_WorkGroupID.y

uniform highp sampler2D depthMap;

layout(std430) buffer tilesBlock
{
	Tile tiles[TILES_Y_COUNT][TILES_X_COUNT];
};

layout(std140) buffer pointLightsBlock
{
	PointLight pointLights[MAX_POINT_LIGHTS];
};

// Common uniforms 
layout(std430) uniform commonBlock
{
	/// Packs:
	/// - xy: Planes. For the calculation of frag pos in view space
	uniform vec2 planes;
};

void main()
{
	//
	// First calculate the z of the near plane
	//
	float minDepth = 10000.0; // min depth of tile

	uvec2 coord = uvec2(uvec2(tileX, tileY) * uvec2(TILE_W, TILE_H));

	for(uint x = 0; x < TILE_W; x++)
	{
		for(uint y = 0; y < TILE_H; i++)
		{
			float depth = texelFetch(depthMap, coord + uvec2(x, y)).r;

			minDepth = min(depth, minDepth);
		}
	}

	// Convert to view space
	float nearZ = -planes.y / (planes.x + minDepth);

	//
	//
	//
	uint newPointLightIndices[MAX_POINT_LIGHTS_PER_TILE];
	uint newPointLightsCount = 0;

	uint pointLightsCount = tiles[vInstanceId].lightsCount[0];
	for(uint i = 0U; i < pointLightsCount; ++i)
	{
		uint lightId = tiles[tileY][tileX].pointLightIndices[i];

		vec4 posRadius = pointLights[lightId].posRadius;
		vec3 pos = posRadius.xyz;
		float radius = posRadius.w;
		
		// Should reject?
		if(pos.z - radius <= nearZ)
		{
			// No

			newPointLightIndices[newPointLightsCount++] = lightId;
		}
	}

	// Copy back
	for(uint i = 0U; i < newPointLightsCount; i++)
	{
		tiles[tileY][tileX].pointLightIndices[i] = newPointLightIndices[i];
	}
	tiles[vInstanceId].lightsCount[0] = newPointLightsCount;
}
