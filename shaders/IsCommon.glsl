// Contains common structures for IS

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

struct Tile
{
	uvec4 lightsCount;
#if __VERSION__ == 430
	uint pointLightIndices[MAX_POINT_LIGHTS_PER_TILE];
	uint spotLightIndices[MAX_SPOT_LIGHTS_PER_TILE];
	uint spotLightTexIndices[MAX_SPOT_LIGHTS_TEX_PER_TILE];
#else
	uvec4 pointLightIndices[MAX_POINT_LIGHTS_PER_TILE / 4];
	uvec4 spotLightIndices[MAX_SPOT_LIGHTS_PER_TILE / 4];
	uvec4 spotLightTexIndices[MAX_SPOT_LIGHTS_TEX_PER_TILE / 4];
#endif
};

struct Light
{
	vec4 posRadius; // xyz: Light pos in eye space. w: The radius
	vec4 diffuseColorShadowmapId;
	vec4 specularColorTexId;
};

#define PointLight Light

struct SpotLight
{
	Light light;
	vec4 lightDirection;
	vec4 outerCosInnerCos;
	mat4 texProjectionMat;
	vec4 extendPoints[4];
};

struct Lights
{
	uvec4 count;
	PointLight pointLights[MAX_POINT_LIGHTS];
	SpotLight spotLights[MAX_SPOT_LIGHTS];
};
