// Contains common structures for IS

// Plane
struct Plane
{
	vec4 normalOffset;
};

// Contains the plane grid
struct Tilegrid
{
	Plane planesX[TILES_X_COUNT - 1];
	Plane planesY[TILES_Y_COUNT - 1];
	Plane planesNear[TILES_COUNT];
	Plane planesFar[TILES_COUNT];
};

// Representation of a tile
struct Tile
{
	uvec4 lightsCount;
#if __VERSION__ > 430
	uint pointLightIndices[MAX_POINT_LIGHTS_PER_TILE];
	uint spotLightIndices[MAX_SPOT_LIGHTS_PER_TILE];
	uint spotTexLightndices[MAX_SPOT_TEX_LIGHTS_PER_TILE];
#else
	uvec4 pointLightIndices[MAX_POINT_LIGHTS_PER_TILE / 4];
	uvec4 spotLightIndices[MAX_SPOT_LIGHTS_PER_TILE / 4];
	uvec4 spotTexLightIndices[MAX_SPOT_TEX_LIGHTS_PER_TILE / 4];
#endif
};

// The base of all lights
struct Light
{
	vec4 posRadius; // xyz: Light pos in eye space. w: The -1/radius
	vec4 diffuseColorShadowmapId; // xyz: diff color, w: shadowmap tex ID
	vec4 specularColorTexId; // xyz: spec color, w: diffuse tex ID
};

// Point light
#define PointLight Light

// Spot light
struct SpotLight
{
	Light lightBase;
	vec4 lightDir;
	vec4 outerCosInnerCos;
	vec4 extendPoints[4]; // The positions of the 4 camera points
};

// Spot light with texture
struct SpotTexLight
{
	SpotLight spotLightBase;
	mat4 texProjectionMat;
};

// A container of many lights
struct Lights
{
	uvec4 count; // x: points, z: 
	PointLight pointLights[MAX_POINT_LIGHTS];
	SpotLight spotLights[MAX_SPOT_LIGHTS];
	SpotTexLight spotTexLights[MAX_SPOT_TEX_LIGHTS];
};
