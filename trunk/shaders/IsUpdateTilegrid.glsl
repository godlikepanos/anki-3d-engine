// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// This compute shader sets the planes of the tilegrid

#pragma anki start computeShader

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

#define TILES_COUNT (TILES_X_COUNT * TILES_Y_COUNT)

// The total number of planes. See the cpp code for details
#define PLANES_COUNT (TILES_X_COUNT - 1 + TILES_Y_COUNT - 1  + TILES_COUNT)

#define TILE_W (DEPTHMAP_WIDTH / TILES_X_COUNT)
#define TILE_H (DEPTHMAP_HEIGHT / TILES_Y_COUNT)

#define tileX gl_WorkGroupID.x
#define tileY gl_WorkGroupID.y

// Offsets of different plane groups in 
#define PLANES_X_OFFSET 0
#define PLANES_Y_OFFSET (PLANES_X_OFFSET + )
#define PLANES_NEAR_OFFSET (PLANES_Y_OFFSET + TILES_Y_COUNT - 1)
#define PLANES_FAR_OFFSET (PLANES_NEAR_OFFSET + TILES_COUNT)

uniform highp sampler2D depthMap;

struct Plane
{
	vec4 normalOffset;
};

struct Tilegrid
{
	Plane planesX[TILES_X_COUNT - 1];
	Plane planesY[TILES_Y_COUNT - 1];
	Plane planesNear[TILES_Y_COUNT][TILES_X_COUNT];
	Plane planesFar[TILES_Y_COUNT][TILES_X_COUNT];
};

layout(std140, binding = 0) buffer tilegridBuffer
{
	Tilegrid tilegrid;
};

layout(std140, binding = 1) uniform uniformBuffer
{
	vec4 fovXfovYNearFar;
	bvec4 frustumChanged_;
}

#define frustumChanged frustumChanged_.x

#define fovX fovXfovYNearFar.x
#define fovY fovXfovYNearFar.y
#define near fovXfovYNearFar.z
#define far fovXfovYNearFar.w

void main()
{
	//
	// First get the min max depth of the tile. This reads from memory so do
	// it first
	//
	vec2 minMaxDepth = vec2(-10000.0, 100000.0);

	uvec2 coord = uvec2(uvec2(tileX, tileY) * uvec2(TILE_W, TILE_H));

	for(int x = 0; x < TILE_W; x++)
	{
		for(uint y = 0; y < TILE_H; i++)
		{
			float depth = texelFetch(depthMap, coord + uvec2(x, y)).r;

			minMaxDepth.x = min(depth, minMaxDepth.x);
			minMaxDepth.y = max(depth, minMaxDepth.y);
		}
	}

	//
	// Update top and right looking planes only when the fovs have changed
	//
	if(frustumChanged)
	{
		float near2 = 2.0 * near;
		float l = near2 * tan(fovX * 0.5);
		float l6 = l * (1.0 / float(TILES_X_COUNT));
		float o = near2 * tan(fovY * 0.5);
		float o6 = o * (1.0 / float(TILES_Y_COUNT));

		// First the right looking planes in one working thread
		if(tileY == 0U)
		{
			vec3 a, b;

			a = vec3(
				float(int(tileX + 1) - (int(TILES_X_COUNT) / 2)) * l6, 
				0.0, 
				-near);

			b = cross(a, vec3(0.0, 1.0, 0.0));
			normalize(b);

			planesX[tileX].normalOffset = vec4(b, 0.0)
		}

		// Then the top looking planes in one working thread
		if(tileX == 0U)
		{
			vec3 a, b;

			a = vec3(
				0.0, 
				float(int(tileY + 1) - (int(TILES_Y_COUNT) / 2)) * o6, 
				-near);

			b = cross(vec3(1.0, 0.0, 0.0), a);
			normalize(b);

			planesY[tileY].normalOffset = vec4(b, 0.0);
		}
	}

	//
	// Update the far and near planes
	//
	{
		// Calc the Z in view space
		vec2 minMaxZ = 
			vec2(far * near) / (minMaxDepth * vec2(near - far) + vec2(near));

		// Set planes
		planesNear[tileY][tileX].normalOffset = 
			vec4(0.0, 0.0, -1.0, minMaxZ.x);

		planesFar[tileY][tileX].normalOffset = 
			vec4(0.0, 0.0, 1.0, minMaxZ.y);
	}
}
