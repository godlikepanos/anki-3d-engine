// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <shaders/MsBsCommon.glsl>

layout(vertices = 3) out;

// Defines
#define IID gl_InvocationID
#define IN_POS4(i_) gl_in[i_].gl_Position
#define IN_POS3(i_) gl_in[i_].gl_Position.xyz
#define OUT_POS4(i_) gl_out[i_].gl_Position

//
// In
//
in gl_PerVertex
{
	Vec4 gl_Position;
}
gl_in[];

layout(location = 0) in Vec2 inTexCoords[];
layout(location = 1) in mediump Vec3 inNormal[];
#if PASS == COLOR
layout(location = 2) in mediump Vec4 inTangent[];
#endif
#if INSTANCE_ID_FRAGMENT_SHADER
layout(location = 3) flat in U32 inInstanceId[];
#endif

//
// Out
//

out gl_PerVertex
{
	Vec4 gl_Position;
}
gl_out[];

layout(location = 0) out Vec2 outTexCoord[];
layout(location = 1) out Vec3 outNormal[];
#if PASS == COLOR
layout(location = 2) out Vec4 outTangent[];
#endif

#if INSTANCE_ID_FRAGMENT_SHADER
struct CommonPatch
{
	U32 instanceId;
};
#endif

struct PNPatch
{
	Vec3 pos021;
	Vec3 pos012;
	Vec3 pos102;
	Vec3 pos201;
	Vec3 pos210;
	Vec3 pos120;
	Vec3 pos111;
};

struct PhongPatch
{
	Vec3 terms[3];
};

out patch PNPatch pnPatch;
out patch PhongPatch phongPatch;
#if INSTANCE_ID_FRAGMENT_SHADER
out patch CommonPatch commonPatch;
#endif

// Project point to plane
Vec3 projectToPlane(Vec3 point, Vec3 planePoint, Vec3 planeNormal)
{
	Vec3 v = point - planePoint;
	F32 pen = dot(v, planeNormal);
	Vec3 d = pen * planeNormal;
	return (point - d);
}

// Calculate control points
void calcPositions()
{
	// The original vertices stay the same
	Vec3 pos030 = IN_POS3(0);
	Vec3 pos003 = IN_POS3(1);
	Vec3 pos300 = IN_POS3(2);

	OUT_POS4(0) = IN_POS4(0);
	OUT_POS4(1) = IN_POS4(1);
	OUT_POS4(2) = IN_POS4(2);

	// edges are names according to the opposing vertex
	Vec3 edgeB300 = pos003 - pos030;
	Vec3 edgeB030 = pos300 - pos003;
	Vec3 edgeB003 = pos030 - pos300;

	// Generate two midpoints on each edge
	pnPatch.pos021 = pos030 + edgeB300 / 3.0;
	pnPatch.pos012 = pos030 + edgeB300 * 2.0 / 3.0;
	pnPatch.pos102 = pos003 + edgeB030 / 3.0;
	pnPatch.pos201 = pos003 + edgeB030 * 2.0 / 3.0;
	pnPatch.pos210 = pos300 + edgeB003 / 3.0;
	pnPatch.pos120 = pos300 + edgeB003 * 2.0 / 3.0;

	pnPatch.pos021 = projectToPlane(pnPatch.pos021, pos030, outNormal[0]);
	pnPatch.pos012 = projectToPlane(pnPatch.pos012, pos003, outNormal[1]);
	pnPatch.pos102 = projectToPlane(pnPatch.pos102, pos003, outNormal[1]);
	pnPatch.pos201 = projectToPlane(pnPatch.pos201, pos300, outNormal[2]);
	pnPatch.pos210 = projectToPlane(pnPatch.pos210, pos300, outNormal[2]);
	pnPatch.pos120 = projectToPlane(pnPatch.pos120, pos030, outNormal[0]);

	// Handle the center
	Vec3 center = (pos003 + pos030 + pos300) / 3.0;
	pnPatch.pos111 =
		(pnPatch.pos021 + pnPatch.pos012 + pnPatch.pos102 + pnPatch.pos201 + pnPatch.pos210 + pnPatch.pos120) / 6.0;
	pnPatch.pos111 += (pnPatch.pos111 - center) / 2.0;
}

Vec3 calcFaceNormal(in Vec3 v0, in Vec3 v1, in Vec3 v2)
{
	return normalize(cross(v1 - v0, v2 - v0));
}

F32 calcEdgeTessLevel(in Vec3 n0, in Vec3 n1, in F32 maxTessLevel)
{
	Vec3 norm = normalize(n0 + n1);
	F32 tess = (1.0 - norm.z) * (maxTessLevel - 1.0) + 1.0;
	return tess;
}

/*F32 calcEdgeTessLevel(in Vec2 p0, in Vec2 p1, in F32 maxTessLevel)
{
	F32 dist = distance(p0, p1) * 10.0;
	return dist * (maxTessLevel - 1.0) + 1.0;
}*/

// Given the face positions in NDC caclulate if the face is front facing or not
Bool isFaceFrontFacing(in Vec2 posNdc[3])
{
	Vec2 a = posNdc[1] - posNdc[0];
	Vec2 b = posNdc[2] - posNdc[1];
	Vec2 c = a.xy * b.yx;
	return (c.x - c.y) > 0.0;
}

// Check if a single NDC position is outside the clip space
Bool posOutsideClipSpace(in Vec2 posNdc)
{
	bvec2 compa = lessThan(posNdc, Vec2(-1.0));
	bvec2 compb = greaterThan(posNdc, Vec2(1.0));
	return all(bvec4(compa, compb));
}

// Check if a face in NDC is outside the clip space
Bool isFaceOutsideClipSpace(in Vec2 posNdc[3])
{
	return any(bvec3(posOutsideClipSpace(posNdc[0]), posOutsideClipSpace(posNdc[1]), posOutsideClipSpace(posNdc[2])));
}

// Check if a face is visible
Bool isFaceVisible(in Mat4 mvp)
{
	// Calculate clip positions
	Vec2 clip[3];
	for(I32 i = 0; i < 3; i++)
	{
		Vec4 v = mvp * IN_POS4(i);
		clip[i] = v.xy / (v.w * 0.5 + 0.5);
	}

	// Check the face orientation and clipping
	return isFaceFrontFacing(clip) && !isFaceOutsideClipSpace(clip);
}

void setSilhouetteTessLevels(in Mat3 normalMat, in F32 maxTessLevel)
{
	// Calculate the normals in view space
	Vec3 nv[3];
	for(I32 i = 0; i < 3; i++)
	{
		nv[i] = normalMat * inNormal[i];
	}

	gl_TessLevelOuter[0] = calcEdgeTessLevel(nv[1], nv[2], maxTessLevel);
	gl_TessLevelOuter[1] = calcEdgeTessLevel(nv[2], nv[0], maxTessLevel);
	gl_TessLevelOuter[2] = calcEdgeTessLevel(nv[0], nv[1], maxTessLevel);
	gl_TessLevelInner[0] = (gl_TessLevelOuter[0] + gl_TessLevelOuter[1] + gl_TessLevelOuter[2]) / 3.0;
}

void setConstantTessLevels(in F32 maxTessLevel)
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

// Used in phong method
F32 calcPhongTerm(I32 ivId, I32 i, Vec3 q)
{
	Vec3 qMinusP = q - IN_POS3(i);
	return q[ivId] - dot(qMinusP, inNormal[i]) * inNormal[i][ivId];
}

// This function is part of the point-normal tessellation method
#define tessellatePNPositionNormalTangentTexCoord_DEFINED
void tessellatePNPositionNormalTangentTexCoord(in F32 maxTessLevel, in Mat4 mvp, in Mat3 normalMat)
{
	F32 tessLevel = 0.0;

	// Calculate the face normal in view space
	Vec3 faceNorm = calcFaceNormal(IN_POS3(0), IN_POS3(1), IN_POS3(2));
	faceNorm = (normalMat * faceNorm);

	if(faceNorm.z >= 0.0)
	{
		// The face is front facing

		for(I32 i = 0; i < 3; i++)
		{
			outTexCoord[i] = inTexCoords[i];
			outNormal[i] = inNormal[i];
#if PASS == COLOR
			outTangent[i] = inTangent[i];
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
void tessellatePhongPositionNormalTangentTexCoord(in F32 maxTessLevel, in Mat4 mvp, in Mat3 normalMat)
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

	OUT_POS4(IID) = IN_POS4(IID); // Do that here to trick the barrier

	barrier();

	if(gl_TessLevelOuter[0] > 0.0)
	{
		outTexCoord[IID] = inTexCoords[IID];
		outNormal[IID] = inNormal[IID];
#if PASS == COLOR
		outTangent[IID] = inTangent[IID];
#endif

		phongPatch.terms[IID][0] = calcPhongTerm(IID, 0, IN_POS3(1)) + calcPhongTerm(IID, 1, IN_POS3(0));
		phongPatch.terms[IID][1] = calcPhongTerm(IID, 1, IN_POS3(2)) + calcPhongTerm(IID, 2, IN_POS3(1));
		phongPatch.terms[IID][2] = calcPhongTerm(IID, 2, IN_POS3(0)) + calcPhongTerm(IID, 0, IN_POS3(2));
	}
}

#define tessellateDispMapPositionNormalTangentTexCoord_DEFINED
void tessellateDispMapPositionNormalTangentTexCoord(in F32 maxTessLevel, in Mat4 mvp, in Mat3 normalMat)
{
	if(IID == 0)
	{
		if(isFaceVisible(mvp))
		{
			setConstantTessLevels(maxTessLevel);

#if INSTANCE_ID_FRAGMENT_SHADER
			commonPatch.instanceId = inInstanceId[0];
#endif
		}
		else
		{
			discardPatch();
		}
	}

	// Passthrough
	OUT_POS4(IID) = IN_POS4(IID);
	outTexCoord[IID] = inTexCoords[IID];
	outNormal[IID] = inNormal[IID];
#if PASS == COLOR
	outTangent[IID] = inTangent[IID];
#endif
}
