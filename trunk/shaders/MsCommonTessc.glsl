#pragma anki include "shaders/MsBsCommon.glsl"

layout(vertices = 3) out;

#define IID gl_InvocationID

// In
in vec3 vPosition[];
in vec2 vTexCoords[];
in mediump vec3 vNormal[];
#if PASS_COLOR
in mediump vec4 vTangent[];
#endif
#if INSTANCE_ID_FRAGMENT_SHADER
flat in uint vInstanceId[];
#endif

// Out
out vec3 tcPosition[];
out vec2 tcTexCoord[];
out vec3 tcNormal[];
#if PASS_COLOR
out vec4 tcTangent[];
#endif

#if INSTANCE_ID_FRAGMENT_SHADER
struct CommonPatch
{
	uint instanceId;
};
#endif

struct PNPatch
{
	vec3 pos021;
	vec3 pos012;
	vec3 pos102;
	vec3 pos201;
	vec3 pos210;
	vec3 pos120;
	vec3 pos111;
};

#define pos030 tcPosition[0]
#define pos003 tcPosition[1]
#define pos300 tcPosition[2]

struct PhongPatch
{
	vec3 terms[3];
};

out patch PNPatch pnPatch;
out patch PhongPatch phongPatch;
#if INSTANCE_ID_FRAGMENT_SHADER
out patch CommonPatch commonPatch;
#endif

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
	pos030 = vPosition[0];
	pos003 = vPosition[1];
	pos300 = vPosition[2];

	// edges are names according to the opposing vertex
	vec3 edgeB300 = pos003 - pos030;
	vec3 edgeB030 = pos300 - pos003;
	vec3 edgeB003 = pos030 - pos300;

	// Generate two midpoints on each edge
	pnPatch.pos021 = pos030 + edgeB300 / 3.0;
	pnPatch.pos012 = pos030 + edgeB300 * 2.0 / 3.0;
	pnPatch.pos102 = pos003 + edgeB030 / 3.0;
	pnPatch.pos201 = pos003 + edgeB030 * 2.0 / 3.0;
	pnPatch.pos210 = pos300 + edgeB003 / 3.0;
	pnPatch.pos120 = pos300 + edgeB003 * 2.0 / 3.0;

	pnPatch.pos021 = projectToPlane(
		pnPatch.pos021, pos030, tcNormal[0]);
	pnPatch.pos012 = projectToPlane(
		pnPatch.pos012, pos003, tcNormal[1]);
	pnPatch.pos102 = projectToPlane(
		pnPatch.pos102, pos003, tcNormal[1]);
	pnPatch.pos201 = projectToPlane(
		pnPatch.pos201, pos300, tcNormal[2]);
	pnPatch.pos210 = projectToPlane(
		pnPatch.pos210, pos300, tcNormal[2]);
	pnPatch.pos120 = projectToPlane(
		pnPatch.pos120, pos030, tcNormal[0]);

	// Handle the center
	vec3 center = (pos003 + pos030 + pos300) / 3.0;
	pnPatch.pos111 = (pnPatch.pos021 + pnPatch.pos012 + pnPatch.pos102 +
		pnPatch.pos201 + pnPatch.pos210 + pnPatch.pos120) / 6.0;
	pnPatch.pos111 += (pnPatch.pos111 - center) / 2.0;
} 

vec3 calcFaceNormal(in vec3 v0, in vec3 v1, in vec3 v2)
{
	return normalize(cross(v1 - v0, v2 - v0));
}

float calcEdgeTessLevel(in vec3 n0, in vec3 n1, in float maxTessLevel)
{
	vec3 norm = normalize(n0 + n1);
	float tess = (1.0 - norm.z) * (maxTessLevel - 1.0) + 1.0;
	return tess;
}

/*float calcEdgeTessLevel(in vec2 p0, in vec2 p1, in float maxTessLevel)
{
	float dist = distance(p0, p1) * 10.0;
	return dist * (maxTessLevel - 1.0) + 1.0;
}*/

// Given the face positions in NDC caclulate if the face is front facing or not
bool isFaceFrontFacing(in vec2 posNdc[3])
{
	vec2 a = posNdc[1] - posNdc[0];
	vec2 b = posNdc[2] - posNdc[1];
	vec2 c = a.xy * b.yx;
	return (c.x - c.y) > 0.0;
}

// Check if a single NDC position is outside the clip space
bool posOutsideClipSpace(in vec2 posNdc)
{
	bvec2 compa = lessThan(posNdc, vec2(-1.0));
	bvec2 compb = greaterThan(posNdc, vec2(1.0));
	return all(bvec4(compa, compb));
}

// Check if a face in NDC is outside the clip space
bool isFaceOutsideClipSpace(in vec2 posNdc[3])
{
	return any(bvec3(
		posOutsideClipSpace(posNdc[0]), 
		posOutsideClipSpace(posNdc[1]), 
		posOutsideClipSpace(posNdc[2])));
}

void setSilhouetteTessLevels(in mat3 normalMat, in float maxTessLevel)
{
	// Calculate the normals in view space
	vec3 nv[3];
	for(int i = 0; i < 3; i++)
	{
		nv[i] = normalMat * vNormal[i];
	}

	gl_TessLevelOuter[0] = calcEdgeTessLevel(nv[1], nv[2], maxTessLevel);
	gl_TessLevelOuter[1] = calcEdgeTessLevel(nv[2], nv[0], maxTessLevel);
	gl_TessLevelOuter[2] = calcEdgeTessLevel(nv[0], nv[1], maxTessLevel);
	gl_TessLevelInner[0] = (gl_TessLevelOuter[0] + gl_TessLevelOuter[1]
		+ gl_TessLevelOuter[2]) / 3.0;
}

void setConstantTessLevels(in float maxTessLevel)
{
	gl_TessLevelOuter[0] = maxTessLevel;
	gl_TessLevelOuter[1] = maxTessLevel;
	gl_TessLevelOuter[2] = maxTessLevel;
	gl_TessLevelInner[0] = maxTessLevel;
}

void discardPatch()
{
	gl_TessLevelOuter[0] = 0.0;
	gl_TessLevelOuter[1] = 0.0;
	gl_TessLevelOuter[2] = 0.0;
	gl_TessLevelInner[0] = 0.0;
}

// Check if a face is visible
bool isFaceVisible(in mat4 mvp)
{
	// Calculate clip positions
	vec2 clip[3];
	for(int i = 0 ; i < 3 ; i++) 
	{
		vec4 v = mvp * vec4(vPosition[i], 1.0);
		clip[i] = v.xy / v.w;
	}

	// Check the face orientation and clipping
	return isFaceFrontFacing(clip) && !isFaceOutsideClipSpace(clip);
}

float calcPhongTerm(int ivId, int i, vec3 q)
{
	vec3 qMinusP = q - vPosition[i];
	return q[ivId] - dot(qMinusP, vNormal[i]) * vNormal[i][ivId];
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
			tcTexCoord[i] = vTexCoords[i];
			tcNormal[i] = vNormal[i];
#if PASS_COLOR
			tcTangent[i] = vTangent[i];
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

#define tessellatePhongPositionNormalTangentTexCoord_DEFINED
void tessellatePhongPositionNormalTangentTexCoord(
	in float maxTessLevel,
	in mat4 mvp,
	in mat3 normalMat)
{
	if(IID == 0)
	{
		if(isFaceVisible(mvp))
		{
			setSilhouetteTessLevels(normalMat, maxTessLevel);
		}
		else
		{
			discardPatch();
		}
	}

	tcPosition[IID] = vPosition[IID]; // Do that here to trick the barrier

	barrier();

	if(gl_TessLevelOuter[0] > 0.0)
	{
		tcTexCoord[IID] = vTexCoords[IID];
		tcNormal[IID] = vNormal[IID];
#if PASS_COLOR
		tcTangent[IID] = vTangent[IID];
#endif

		phongPatch.terms[IID][0] = calcPhongTerm(IID, 0, vPosition[1]) 
			+ calcPhongTerm(IID, 1, vPosition[0]);
		phongPatch.terms[IID][1] = calcPhongTerm(IID, 1, vPosition[2]) 
			+ calcPhongTerm(IID, 2, vPosition[1]);
		phongPatch.terms[IID][2] = calcPhongTerm(IID, 2, vPosition[0]) 
			+ calcPhongTerm(IID, 0, vPosition[2]);
	}
}

#define tessellateDispMapPositionNormalTangentTexCoord_DEFINED
void tessellateDispMapPositionNormalTangentTexCoord(
	in float maxTessLevel,
	in mat4 mvp,
	in mat3 normalMat)
{
	if(IID == 0)
	{
		if(isFaceVisible(mvp))
		{
			setSilhouetteTessLevels(normalMat, maxTessLevel);

#if INSTANCE_ID_FRAGMENT_SHADER
			commonPatch.instanceId = vInstanceId[0];
#endif
		}
		else
		{
			discardPatch();
		}
	}

	tcPosition[IID] = vPosition[IID];
	tcTexCoord[IID] = vTexCoords[IID];
	tcNormal[IID] = vNormal[IID];
#if PASS_COLOR
	tcTangent[IID] = vTangent[IID];
#endif
}
