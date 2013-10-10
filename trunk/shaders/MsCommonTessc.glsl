#pragma anki include "shaders/MsBsCommon.glsl"

layout(vertices = 1) out;

// Varyings in
in highp vec3 vPosition[];
in highp vec2 vTexCoords[];
in mediump vec3 vNormal[];
#if PASS_COLOR
in mediump vec4 vTangent[];
#endif

#if 0
// Varyings out
out highp vec3 tcPosition[];
out highp vec2 tcTexCoords[];
#if PASS_COLOR
out mediump vec3 tcNormal[];
out mediump vec4 tcTangent[];
#endif
#endif

struct OutPatch
{
	vec3 pos030;
	vec3 pos021;
	vec3 pos012;
	vec3 pos003;
	vec3 pos102;
	vec3 pos201;
	vec3 pos300;
	vec3 pos210;
	vec3 pos120;
	vec3 pos111;

	vec2 texCoord[3];
	vec3 normal[3];
#if PASS_COLOR
	vec4 tangent[3];
#endif
};

out patch OutPatch tcPatch;

// Project point to plane
vec3 projectToPlane(vec3 point, vec3 planePoint, vec3 planeNormal)
{
	vec3 v = point - planePoint;
	float pen = dot(v, planeNormal);
	vec3 d = pen * planeNormal;
	return (point - d);
} 

// Calculate control points
void calcPositions()
{
	// The original vertices stay the same
	tcPatch.pos030 = vPosition[0];
	tcPatch.pos003 = vPosition[1];
	tcPatch.pos300 = vPosition[2];

	// edges are names according to the opposing vertex
	vec3 edgeB300 = tcPatch.pos003 - tcPatch.pos030;
	vec3 edgeB030 = tcPatch.pos300 - tcPatch.pos003;
	vec3 edgeB003 = tcPatch.pos030 - tcPatch.pos300;

	// Generate two midpoints on each edge
	tcPatch.pos021 = tcPatch.pos030 + edgeB300 / 3.0;
	tcPatch.pos012 = tcPatch.pos030 + edgeB300 * 2.0 / 3.0;
	tcPatch.pos102 = tcPatch.pos003 + edgeB030 / 3.0;
	tcPatch.pos201 = tcPatch.pos003 + edgeB030 * 2.0 / 3.0;
	tcPatch.pos210 = tcPatch.pos300 + edgeB003 / 3.0;
	tcPatch.pos120 = tcPatch.pos300 + edgeB003 * 2.0 / 3.0;

	tcPatch.pos021 = 
		projectToPlane(tcPatch.pos021, tcPatch.pos030, tcPatch.normal[0]);
	tcPatch.pos012 =
		projectToPlane(tcPatch.pos012, tcPatch.pos003, tcPatch.normal[1]);
	tcPatch.pos102 = 
		projectToPlane(tcPatch.pos102, tcPatch.pos003, tcPatch.normal[1]);
	tcPatch.pos201 = 
		projectToPlane(tcPatch.pos201, tcPatch.pos300, tcPatch.normal[2]);
	tcPatch.pos210 = 
		projectToPlane(tcPatch.pos210, tcPatch.pos300, tcPatch.normal[2]);
	tcPatch.pos120 = 
		projectToPlane(tcPatch.pos120, tcPatch.pos030, tcPatch.normal[0]);

	// Handle the center
	vec3 center = (tcPatch.pos003 + tcPatch.pos030 + tcPatch.pos300) / 3.0;
	tcPatch.pos111 = (tcPatch.pos021 + tcPatch.pos012 + tcPatch.pos102 +
		tcPatch.pos201 + tcPatch.pos210 + tcPatch.pos120) / 6.0;
	tcPatch.pos111 += (tcPatch.pos111 - center) / 2.0;
} 

#define tessellatePositionNormalTangentTexCoord_DEFINED
void tessellatePositionNormalTangentTexCoord(
	in float tessLevelInner, 
	in float tessLevelOuter)
{
	for(int i = 0 ; i < 3 ; i++) 
	{		
		tcPatch.texCoord[i] = vTexCoords[i];
		tcPatch.normal[i] = vNormal[i];
#if PASS_COLOR
		tcPatch.tangent[i] = vTangent[i];
#endif
	}

	calcPositions();

	// Calculate the tessellation levels
	gl_TessLevelOuter[0] = tessLevelOuter;
	gl_TessLevelOuter[1] = tessLevelOuter;
	gl_TessLevelOuter[2] = tessLevelOuter;
	gl_TessLevelInner[0] = tessLevelInner;
}

