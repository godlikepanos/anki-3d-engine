// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/MsFsCommon.glsl"

//
// Attributes
//
layout(location = POSITION_LOCATION) in highp vec3 in_position;
layout(location = TEXTURE_COORDINATE_LOCATION) in mediump vec2 in_uv;

#if PASS == COLOR || TESSELLATION
layout(location = NORMAL_LOCATION) in mediump vec4 in_normal;
#endif

#if PASS == COLOR
layout(location = TANGENT_LOCATION) in mediump vec4 in_tangent;
#endif

//
// Varyings
//
out gl_PerVertex
{
	vec4 gl_Position;
};

layout(location = 0) out mediump vec2 out_uv;

#if PASS == COLOR || TESSELLATION
layout(location = 1) out mediump vec3 out_normal;
#endif

#if PASS == COLOR
layout(location = 2) out mediump vec4 out_tangent;

// For env mapping. AKA view vector
layout(location = 3) out mediump vec3 out_vertPosViewSpace;
#endif

#if INSTANCE_ID_FRAGMENT_SHADER
layout(location = 4) flat out uint out_instanceId;
#endif

//==============================================================================
#define writePositionAndUv_DEFINED
void writePositionAndUv(in mat4 mvp)
{
#if PASS == DEPTH && LOD > 0
// No tex coords for you
#else
	out_uv = in_uv;
#endif

#if TESSELLATION
	gl_Position = vec4(in_position, 1.0);
#else
	gl_Position = mvp * vec4(in_position, 1.0);
#endif
}

//==============================================================================
#if PASS == COLOR
#define writeNormalAndTangent_DEFINED
void writeNormalAndTangent(in mat3 normalMat)
{

#if TESSELLATION

	// Passthrough
	out_normal = in_normal.xyz;
#if PASS == COLOR
	out_tangent = in_tangent;
#endif

#else

#if PASS == COLOR
	out_normal = normalMat * in_normal.xyz;
	out_tangent.xyz = normalMat * in_tangent.xyz;
	out_tangent.w = in_tangent.w;
#endif

#endif
}
#endif

//==============================================================================
#if PASS == COLOR
#define writeVertPosViewSpace_DEFINED
void writeVertPosViewSpace(in mat4 modelViewMat)
{
	out_vertPosViewSpace = vec3(modelViewMat * vec4(in_position, 1.0));
}
#endif
