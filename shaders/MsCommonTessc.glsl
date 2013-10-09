#pragma anki include "shaders/MsBsCommon.glsl"

layout(vertices = 3) out;

// Varyings in
in highp vec3 vPosition[];
in highp vec2 vTexCoords[];
#if PASS_COLOR
in mediump vec3 vNormal[];
in mediump vec4 vTangent[];
#endif

// Varyings out
out highp vec3 tcPosition[];
out highp vec2 tcTexCoords[];
#if PASS_COLOR
out mediump vec3 tcNormal[];
out mediump vec4 tcTangent[];
#endif

#define tessellatePositionNormalTangentTexCoord_DEFINED
void tessellatePositionNormalTangentTexCoord(
	in float tessLevelInner, 
	in float tessLevelOuter)
{
	tcPosition[gl_InvocationID] = vPosition[gl_InvocationID];
	tcTexCoords[gl_InvocationID] = vTexCoords[gl_InvocationID];
#if PASS_COLOR
	tcNormal[gl_InvocationID] = vNormal[gl_InvocationID];
	tcTangent[gl_InvocationID] = vTangent[gl_InvocationID];
#endif

	if(gl_InvocationID == 0) 
	{
		gl_TessLevelInner[0] = tessLevelInner;
		gl_TessLevelOuter[0] = tessLevelOuter;
		gl_TessLevelOuter[1] = tessLevelOuter;
		gl_TessLevelOuter[2] = tessLevelOuter;
	}
}

