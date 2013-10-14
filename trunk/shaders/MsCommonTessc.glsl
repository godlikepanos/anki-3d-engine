#pragma anki include "shaders/MsBsCommon.glsl"

layout(vertices = 1) out;

// Varyings in
in highp vec3 vPosition[];
in highp vec2 vTexCoords[];
in mediump vec3 vNormal[];
#if PASS_COLOR
in mediump vec4 vTangent[];
#endif

struct PNPatch
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

struct PhongPatch
{
	vec3 terms[3];

	vec3 positions[3];
	vec2 texCoord[3];
	vec3 normal[3];
#if PASS_COLOR
	vec4 tangent[3];
#endif
};

out patch PNPatch pnPatch;
out patch PhongPatch phongPatch;

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
	pnPatch.pos030 = vPosition[0];
	pnPatch.pos003 = vPosition[1];
	pnPatch.pos300 = vPosition[2];

	// edges are names according to the opposing vertex
	vec3 edgeB300 = pnPatch.pos003 - pnPatch.pos030;
	vec3 edgeB030 = pnPatch.pos300 - pnPatch.pos003;
	vec3 edgeB003 = pnPatch.pos030 - pnPatch.pos300;

	// Generate two midpoints on each edge
	pnPatch.pos021 = pnPatch.pos030 + edgeB300 / 3.0;
	pnPatch.pos012 = pnPatch.pos030 + edgeB300 * 2.0 / 3.0;
	pnPatch.pos102 = pnPatch.pos003 + edgeB030 / 3.0;
	pnPatch.pos201 = pnPatch.pos003 + edgeB030 * 2.0 / 3.0;
	pnPatch.pos210 = pnPatch.pos300 + edgeB003 / 3.0;
	pnPatch.pos120 = pnPatch.pos300 + edgeB003 * 2.0 / 3.0;

	pnPatch.pos021 = 
		projectToPlane(pnPatch.pos021, pnPatch.pos030, pnPatch.normal[0]);
	pnPatch.pos012 =
		projectToPlane(pnPatch.pos012, pnPatch.pos003, pnPatch.normal[1]);
	pnPatch.pos102 = 
		projectToPlane(pnPatch.pos102, pnPatch.pos003, pnPatch.normal[1]);
	pnPatch.pos201 = 
		projectToPlane(pnPatch.pos201, pnPatch.pos300, pnPatch.normal[2]);
	pnPatch.pos210 = 
		projectToPlane(pnPatch.pos210, pnPatch.pos300, pnPatch.normal[2]);
	pnPatch.pos120 = 
		projectToPlane(pnPatch.pos120, pnPatch.pos030, pnPatch.normal[0]);

	// Handle the center
	vec3 center = (pnPatch.pos003 + pnPatch.pos030 + pnPatch.pos300) / 3.0;
	pnPatch.pos111 = (pnPatch.pos021 + pnPatch.pos012 + pnPatch.pos102 +
		pnPatch.pos201 + pnPatch.pos210 + pnPatch.pos120) / 6.0;
	pnPatch.pos111 += (pnPatch.pos111 - center) / 2.0;
} 

vec3 calcFaceNormal(in vec3 v0, in vec3 v1, in vec3 v2)
{
	return normalize(cross(v1 - v0, v2 - v0));
}

// This function is part of the point-normal tessellation method
#define tessellatePNPositionNormalTangentTexCoord_DEFINED
void tessellatePNPositionNormalTangentTexCoord(
	in float maxTessLevel,
	in mat4 mvp,
	in mat3 normalMat)
{
	float tessLevel = 0.0;

	// Calculate the face normal in view space
	vec3 faceNorm = calcFaceNormal(vPosition[0], vPosition[1], vPosition[2]);
	faceNorm = (normalMat * faceNorm);

	if(faceNorm.z >= 0.0)
	{
		// The face is front facing

		for(int i = 0 ; i < 3 ; i++) 
		{		
			pnPatch.texCoord[i] = vTexCoords[i];
			pnPatch.normal[i] = vNormal[i];
#if PASS_COLOR
			pnPatch.tangent[i] = vTangent[i];
#endif
		}

		calcPositions();

		// Calculate the tessLevel. It's 1.0 when the normal is facing the cam
		// and maxTessLevel when it's facing away. This gives high tessellation
		// on silhouettes
		tessLevel = (1.0 - faceNorm.z) * (maxTessLevel - 1.0) + 1.0;
	}

	gl_TessLevelOuter[0] = tessLevel;
	gl_TessLevelOuter[1] = tessLevel;
	gl_TessLevelOuter[2] = tessLevel;
	gl_TessLevelInner[0] = tessLevel;
}

float calcTerm(int ivId, int i, vec3 q)
{
	vec3 qMinusP = q - vPosition[i];
	return q[ivId] - dot(qMinusP, vNormal[i]) * vNormal[i][ivId];
}

#define tessellatePhongPositionNormalTangentTexCoord_DEFINED
void tessellatePhongPositionNormalTangentTexCoord(
	in float maxTessLevel,
	in mat4 mvp,
	in mat3 normalMat)
{
	float tessLevel = 0.0;

	// Calculate the face normal in view space
	vec3 faceNorm = calcFaceNormal(vPosition[0], vPosition[1], vPosition[2]);
	faceNorm = (normalMat * faceNorm);

	if(faceNorm.z >= -0.0)
	{
		// The face is front facing

		for(int i = 0 ; i < 3 ; i++) 
		{
			phongPatch.positions[i] = vPosition[i];
			phongPatch.texCoord[i] = vTexCoords[i];
			phongPatch.normal[i] = vNormal[i];
#if PASS_COLOR
			phongPatch.tangent[i] = vTangent[i];
#endif

			phongPatch.terms[i][0] = 
				calcTerm(i, 0, vPosition[1]) + calcTerm(i, 1, vPosition[0]);
			phongPatch.terms[i][1] = 
				calcTerm(i, 1, vPosition[2]) + calcTerm(i, 2, vPosition[1]);
			phongPatch.terms[i][2] = 
				calcTerm(i, 2, vPosition[0]) + calcTerm(i, 0, vPosition[2]);
		}

		// Calculate the tessLevel. It's 1.0 when the normal is facing the cam
		// and maxTessLevel when it's facing away. This gives high tessellation
		// on silhouettes
		tessLevel = (1.0 - faceNorm.z) * (maxTessLevel - 1.0) + 1.0;
	}

	gl_TessLevelOuter[0] = tessLevel;
	gl_TessLevelOuter[1] = tessLevel;
	gl_TessLevelOuter[2] = tessLevel;
	gl_TessLevelInner[0] = tessLevel;
}

