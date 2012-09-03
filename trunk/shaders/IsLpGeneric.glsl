/// @file
///
/// Illumination stage lighting pass general shader program

#pragma anki start vertexShader

#pragma anki include "shaders/IsLpVertex.glsl"

#pragma anki start fragmentShader

#pragma anki include "shaders/Pack.glsl"

#if !MAX_LIGHTS_PER_TILE || !TILES_X_COUNT || !TILES_Y_COUNT || !TILES_COUNT
#	error "See file"
#endif

/// @name Uniforms
/// @{

// Common uniforms between lights
layout(std140, row_major, binding = 0) uniform commonBlock
{
	/// Packs:
	/// - x: zNear. For the calculation of frag pos in view space
	/// - zw: Planes. For the calculation of frag pos in view space
	uniform vec4 nearPlanes;

	/// For the calculation of frag pos in view space. The xy is the 
	/// limitsOfNearPlane and the zw is an optimization see PpsSsao.glsl and 
	/// r403 for the clean one
	uniform vec4 limitsOfNearPlane_;
};

#define planes nearPlanes.zw
#define zNear nearPlanes.x
#define limitsOfNearPlane limitsOfNearPlane_.xy
#define limitsOfNearPlane2 limitsOfNearPlane_.zw

struct Light
{
	vec4 posAndRadius; ///< xyz: Light pos in eye space. w: The radius
	vec4 diffuseColor;
	vec4 specularColor;
#if SPOT_LIGHT
	mat4 texProjectionMat;
#endif
};

layout(std140, row_major, binding = 1) uniform lightsBlock
{
	Light lights[MAX_LIGHTS];
};

struct Tile
{
	uint lightsCount;
	uvec4 lightIndices[MAX_LIGHTS_PER_TILE / 4];
};

layout(std140, row_major, binding = 2) uniform tilesBlock
{
	Tile tiles[TILES_X_COUNT * TILES_Y_COUNT];
};

uniform usampler2D msFai0;
uniform sampler2D msDepthFai;
uniform sampler2D lightTex;
uniform sampler2DShadow shadowMap;
/// @}

/// @name Varyings
/// @{
in vec2 vTexCoords;
flat in uint vInstanceId;
/// @}

/// @name Output
/// @{
out vec3 fColor;
/// @}

void main()
{
	fColor = vec3(0.0, float(tiles[vInstanceId].lightsCount), 0.0);
}
