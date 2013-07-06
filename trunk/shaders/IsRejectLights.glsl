// This compute shader rejects lights

#pragma anki start computeShader

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

#pragma anki include "shaders/IsCommon.glsl"

#define TILE_W (DEPTHMAP_WIDTH / TILES_X_COUNT)
#define TILE_H (DEPTHMAP_HEIGHT / TILES_Y_COUNT)

#define tileX gl_WorkGroupID.x
#define tileY gl_WorkGroupID.y

uniform highp sampler2D depthMap;

layout(std140, binding = TILES_BLOCK_BINDING) buffer tilesBlock
{
	Tile tiles[TILES_Y_COUNT * TILES_X_COUNT];
};

layout(std140) uniform pointLightsBlock
{
	PointLight pointLights[MAX_POINT_LIGHTS];
};

// Common uniforms. Make it match the commonBlock in IsLp.glsl
layout(std140) uniform commonBlock
{
	/// Packs:
	/// - xy: Planes. For the calculation of frag pos in view space
	uniform vec2 planes;

	uniform vec4 padding1;
	uniform vec4 padding2;
};

void main()
{
	uint tileIndex = tileY * TILES_X_COUNT + tileX;

	//
	// First calculate the z of the far plane
	//
	vec2 minMaxDepth = vec2(10000.0, -10000.0); // max depth of tile

	const vec2 COORD = vec2(tileX, tileY) 
		* (vec2(1.0) / vec2(float(TILES_X_COUNT), float(TILES_Y_COUNT)));

	vec2 coord = COORD;

	for(int x = 0; x < TILE_W; ++x)
	{
		for(int y = 0; y < TILE_H; ++y)
		{
			float depth = texture(depthMap, coord).r;

			minMaxDepth[0] = min(depth, minMaxDepth[0]);
			minMaxDepth[1] = max(depth, minMaxDepth[1]);

			coord.y += 1.0 / float(DEPTHMAP_HEIGHT);
		}

		coord.x += 1.0 / float(DEPTHMAP_WIDTH);
	}

	// Convert to view space
	vec2 z = -planes.y / (planes.x + minMaxDepth);

	//
	// Reject point lights
	//
	uint newPointLightIndices[MAX_POINT_LIGHTS_PER_TILE];
	uint newPointLightsCount = 0;

	uint pointLightsCount = tiles[tileIndex].lightsCount[0];
	for(uint i = 0U; i < pointLightsCount; ++i)
	{
		uint lightId = tiles[tileIndex].pointLightIndices[i / 4U][i % 4U];

		vec4 posRadius = pointLights[lightId].posRadius;
		vec3 pos = posRadius.xyz;
		float radius = -1.0 / posRadius.w;
		
		// Should reject?
		if((pos.z + radius) >= z[1] && (pos.z - radius) <= z[0])
		{
			// No

			newPointLightIndices[newPointLightsCount++] = lightId;
		}
	}

	// Copy back
	for(uint i = 0U; i < newPointLightsCount; i++)
	{
		tiles[tileIndex].pointLightIndices[i / 4U][i % 4U] = 
			newPointLightIndices[i];
	}
	tiles[tileIndex].lightsCount[0] = newPointLightsCount;
}
