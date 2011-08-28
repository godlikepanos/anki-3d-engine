/// @name Uniforms
/// @{
uniform float blurring;
/// @}

/// @name Varyings
/// @{
in vec2 vTexCoords;
in vec3 vNormal;
in vec3 vTangent;
in float vTangentW;
in vec3 vVertPosViewSpace;
/// @}

/// @name Fragment out
/// @{
#if defined(COLOR_PASS)
layout(location = 0) out vec3 fMsNormalFai;
layout(location = 1) out vec3 fMsDiffuseFai;
layout(location = 2) out vec4 fMsSpecularFai;
#endif
/// @}
