/// @file
///
/// Illumination stage lighting pass general shader program

#pragma anki start vertexShader

#pragma anki include "shaders/IsLpVertex.glsl"

#pragma anki start fragmentShader

#pragma anki include "shaders/Pack.glsl"

/// @name Uniforms
/// @{

struct Light
{
	vec4 posAndRadius; ///< xyz: Light pos in eye space. w: The radius
	vec4 diffuseColor;
	vec4 specularColor;
#if SPOT_LIGHT
	mat4 texProjectionMat;
#endif
};

// Common uniforms between lights
layout(std140, row_major) uniform generalBlock
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

uniform usampler2D msFai0;
uniform sampler2D msDepthFai;
uniform sampler2D lightTex;
uniform sampler2DShadow shadowMap;
/// @}

/// @name Varyings
/// @{
in vec2 vTexCoords;
/// @}

/// @name Output
/// @{
out vec3 fColor;
/// @}

void main()
{
	fColor = vec3(vTexCoords, 0.0);
}
