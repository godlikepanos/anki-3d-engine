#pragma anki include "shaders/MsBsCommon.glsl"

//
// Attributes
//
layout(location = POSITION_LOCATION) in highp vec3 inPosition;
layout(location = TEXTURE_COORDINATE_LOCATION) in mediump vec2 inTexCoord;

#if PASS == COLOR || TESSELLATION
layout(location = NORMAL_LOCATION) in mediump vec3 inNormal;
#endif

#if PASS == COLOR
layout(location = TANGENT_LOCATION) in mediump vec4 inTangent;
#endif

//
// Varyings
//
out gl_PerVertex
{
	vec4 gl_Position;
};

layout(location = 0) out mediump vec2 outTexCoords;

#if PASS == COLOR || TESSELLATION
layout(location = 1) out mediump vec3 outNormal;
#endif

#if PASS == COLOR
layout(location = 2) out mediump vec4 outTangent;

// For env mapping. AKA view vector
layout(location = 3) out mediump vec3 outVertPosViewSpace; 
#endif

#if INSTANCE_ID_FRAGMENT_SHADER
layout(location = 4) flat out uint outInstanceId;
#endif

//==============================================================================
#define writePositionTexCoord_DEFINED
void writePositionTexCoord(in mat4 mvp)
{
#if PASS == DEPTH && LOD > 0
	// No tex coords for you
#else
	outTexCoords = inTexCoord;
#endif

#if TESSELLATION
	gl_Position = vec4(inPosition, 1.0);
#else
	gl_Position = mvp * vec4(inPosition, 1.0);
#endif
}

//==============================================================================
#define writePositionNormalTangentTexCoord_DEFINED
void writePositionNormalTangentTexCoord(in mat4 mvp, in mat3 normalMat)
{

#if TESSELLATION

	// Passthrough
	outNormal = inNormal;
#	if PASS == COLOR
	outTangent = inTangent;
#	endif

#else

#	if PASS == COLOR
	outNormal = normalMat * inNormal;
	outTangent.xyz = normalMat * inTangent.xyz;
	outTangent.w = inTangent.w;
#	endif

#endif

	writePositionTexCoord(mvp);
}

//==============================================================================
#if PASS == COLOR
#define writeVertPosViewSpace_DEFINED
void writeVertPosViewSpace(in mat4 modelViewMat)
{
	outVertPosViewSpace = vec3(modelViewMat * vec4(inPosition, 1.0));
}
#endif
