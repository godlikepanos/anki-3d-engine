// Contains common structures for IS

// Representation of a tile
struct Tile
{
	uvec4 lightsCount;
	uint pointLightIndices[MAX_POINT_LIGHTS_PER_TILE];
	uint spotLightIndices[MAX_SPOT_LIGHTS_PER_TILE];
	uint spotTexLightIndices[MAX_SPOT_TEX_LIGHTS_PER_TILE];
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

// Common uniforms between lights
layout(std140, row_major, binding = 0) readonly buffer commonBlock
{
	/// Packs:
	/// - xy: Planes. For the calculation of frag pos in view space
	vec4 uPlanesComp;

	vec4 uSceneAmbientColor;

	vec4 uGroundLightDir;

	vec4 uLimitsOfNearPlane;
};

#define uPlanes uPlanesComp.xy
