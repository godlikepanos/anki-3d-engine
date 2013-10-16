#pragma anki include "shaders/MsBsCommon.glsl"

layout(vertices = 1) out;

// Varyings in
in highp vec3 vPosition[];
in highp vec2 vTexCoords[];
in mediump vec3 vNormal[];
#if PASS_COLOR
in mediump vec4 vTangent[];
#endif
#if INSTANCE_ID_FRAGMENT_SHADER
uint vInstanceId[];
#endif

// Out
struct CommonPatch
{
	vec3 positions[3];
	vec2 texCoord[3];
	vec3 normal[3];
#if PASS_COLOR
	vec4 tangent[3];
#endif
#if INSTANCE_ID_FRAGMENT_SHADER
	uint instanceId;
#endif
};

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

#define pos030 positions[0]
#define pos003 positions[1]
#define pos300 positions[2]

struct PhongPatch
{
	vec3 terms[3];
};

out patch PNPatch pnPatch;
out patch PhongPatch phongPatch;
out patch CommonPatch commonPatch;

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
	commonPatch.pos030 = vPosition[0];
	commonPatch.pos003 = vPosition[1];
	commonPatch.pos300 = vPosition[2];

	// edges are names according to the opposing vertex
	vec3 edgeB300 = commonPatch.pos003 - commonPatch.pos030;
	vec3 edgeB030 = commonPatch.pos300 - commonPatch.pos003;
	vec3 edgeB003 = commonPatch.pos030 - commonPatch.pos300;

	// Generate two midpoints on each edge
	pnPatch.pos021 = commonPatch.pos030 + edgeB300 / 3.0;
	pnPatch.pos012 = commonPatch.pos030 + edgeB300 * 2.0 / 3.0;
	pnPatch.pos102 = commonPatch.pos003 + edgeB030 / 3.0;
	pnPatch.pos201 = commonPatch.pos003 + edgeB030 * 2.0 / 3.0;
	pnPatch.pos210 = commonPatch.pos300 + edgeB003 / 3.0;
	pnPatch.pos120 = commonPatch.pos300 + edgeB003 * 2.0 / 3.0;

	pnPatch.pos021 = projectToPlane(
		pnPatch.pos021, commonPatch.pos030, commonPatch.normal[0]);
	pnPatch.pos012 = projectToPlane(
		pnPatch.pos012, commonPatch.pos003, commonPatch.normal[1]);
	pnPatch.pos102 = projectToPlane(
		pnPatch.pos102, commonPatch.pos003, commonPatch.normal[1]);
	pnPatch.pos201 = projectToPlane(
		pnPatch.pos201, commonPatch.pos300, commonPatch.normal[2]);
	pnPatch.pos210 = projectToPlane(
		pnPatch.pos210, commonPatch.pos300, commonPatch.normal[2]);
	pnPatch.pos120 = projectToPlane(
		pnPatch.pos120, commonPatch.pos030, commonPatch.normal[0]);

	// Handle the center
	vec3 center = (commonPatch.pos003 + commonPatch.pos030 
		+ commonPatch.pos300) / 3.0;
	pnPatch.pos111 = (pnPatch.pos021 + pnPatch.pos012 + pnPatch.pos102 +
		pnPatch.pos201 + pnPatch.pos210 + pnPatch.pos120) / 6.0;
	pnPatch.pos111 += (pnPatch.pos111 - center) / 2.0;
} 

vec3 calcFaceNormal(in vec3 v0, in vec3 v1, in vec3 v2)
{
	return normalize(cross(v1 - v0, v2 - v0));
}

/*float calcEdgeTessLevel(in vec3 n0, in vec3 n1, in float maxTessLevel)
{
	vec3 norm = normalize(n0 + n1);
	return (1.0 - norm.z) * (maxTessLevel - 1.0) + 1.0;
}*/

float calcEdgeTessLevel(in vec2 p0, in vec2 p1, in float maxTessLevel)
{
	float dist = distance(p0, p1) * 10.0;
	return dist * (maxTessLevel - 1.0) + 1.0;
}

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
			commonPatch.texCoord[i] = vTexCoords[i];
			commonPatch.normal[i] = vNormal[i];
#if PASS_COLOR
			commonPatch.tangent[i] = vTangent[i];
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
	// Calculate clip positions
	vec2 clip[3];
	for(int i = 0 ; i < 3 ; i++) 
	{
		vec4 v = mvp * vec4(vPosition[i], 1.0);
		clip[i] = v.xy / v.w;
	}

	// Check the face orientation and clipping
	if(isFaceFrontFacing(clip) && !isFaceOutsideClipSpace(clip))
	{
		// The face is front facing and inside the clip space

		for(int i = 0 ; i < 3 ; i++) 
		{
			commonPatch.positions[i] = vPosition[i];
			commonPatch.texCoord[i] = vTexCoords[i];
			commonPatch.normal[i] = vNormal[i];
#if PASS_COLOR
			commonPatch.tangent[i] = vTangent[i];
#endif

			phongPatch.terms[i][0] = 
				calcTerm(i, 0, vPosition[1]) + calcTerm(i, 1, vPosition[0]);
			phongPatch.terms[i][1] = 
				calcTerm(i, 1, vPosition[2]) + calcTerm(i, 2, vPosition[1]);
			phongPatch.terms[i][2] = 
				calcTerm(i, 2, vPosition[0]) + calcTerm(i, 0, vPosition[2]);
		}

		// Calculate the normals in view space
		vec3 nv[3];
		for(int i = 0; i < 3; i++)
		{
			nv[i] = normalMat * vNormal[i];
		}

		gl_TessLevelOuter[0] = maxTessLevel;
		gl_TessLevelOuter[1] = maxTessLevel;
		gl_TessLevelOuter[2] = maxTessLevel;
		gl_TessLevelInner[0] = maxTessLevel;
	}
	else
	{
		gl_TessLevelOuter[0] = 0.0;
		gl_TessLevelOuter[1] = 0.0;
		gl_TessLevelOuter[2] = 0.0;
		gl_TessLevelInner[0] = 0.0;
	}
}


#define tessellateDispMapPositionNormalTangentTexCoord_DEFINED
void tessellateDispMapPositionNormalTangentTexCoord(
	in float maxTessLevel,
	in mat4 mvp,
	in mat3 normalMat)
{
	for(int i = 0 ; i < 3 ; i++) 
	{
		commonPatch.positions[i] = vPosition[i];
		commonPatch.texCoord[i] = vTexCoords[i];
		commonPatch.normal[i] = vNormal[i];
#if PASS_COLOR
		commonPatch.tangent[i] = vTangent[i];
#endif
	}

#if INSTANCE_ID_FRAGMENT_SHADER
	commonPatch.instanceId = vInstanceId[0];
#endif

	gl_TessLevelOuter[0] = maxTessLevel;
	gl_TessLevelOuter[1] = maxTessLevel;
	gl_TessLevelOuter[2] = maxTessLevel;
	gl_TessLevelInner[0] = maxTessLevel;
}
