#pragma anki start computeShader

// Set workgroup XXX
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

struct Tile
{
	uvec4 lightsCount;
	uvec4 lightIndices[MAX_LIGHTS_PER_TILE / 4];
};

layout(std140, binding = 0) buffer tilesBuffer
{
	Tile tiles[TILES_X_COUNT * TILES_Y_COUNT];
};

void main()
{
	tiles[gl_WorkGroupID.x * TILES_Y_COUNT + gl_WorkGroupID.y] = 0.1;
}
