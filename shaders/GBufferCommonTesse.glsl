// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

layout(triangles, equal_spacing, ccw) in;

#define IN_POS4(i_) gl_in[i_].gl_Position
#define IN_POS3(i_) gl_in[i_].gl_Position.xyz

//
// Input
//

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

#if INSTANCE_ID_FRAGMENT_SHADER
struct CommonPatch
{
	U32 instanceId;
};
#endif

in gl_PerVertex
{
	Vec4 gl_Position;
}
gl_in[];

in patch PNPatch pnPatch;
in patch PhongPatch phongPatch;
#if INSTANCE_ID_FRAGMENT_SHADER
in patch CommonPatch commonPatch;
#endif

layout(location = 0) in Vec2 inTexCoord[];
layout(location = 1) in Vec3 inNormal[];
#if PASS == COLOR
layout(location = 2) in Vec4 inTangent[];
#endif

//
// Output
//

out gl_PerVertex
{
	Vec4 gl_Position;
};

layout(location = 0) out highp Vec2 outTexCoord;
#if PASS == COLOR
layout(location = 1) out mediump Vec3 outNormal;
layout(location = 2) out mediump Vec4 outTangent;
#endif

#define INTERPOLATE(x_) (x_[0] * gl_TessCoord.x + x_[1] * gl_TessCoord.y + x_[2] * gl_TessCoord.z)

// Smooth tessellation
#define tessellatePNPositionNormalTangentTexCoord_DEFINED
void tessellatePNPositionNormalTangentTexCoord(in Mat4 mvp, in Mat3 normalMat)
{
#if PASS == COLOR
	outNormal = normalize(normalMat * INTERPOLATE(inNormal));
	outTangent = INTERPOLATE(inTangent);
	outTangent.xyz = normalize(normalMat * outTangent.xyz);
#endif

	outTexCoord = INTERPOLATE(inTexCoord);

	F32 u = gl_TessCoord.x;
	F32 v = gl_TessCoord.y;
	F32 w = gl_TessCoord.z;

	F32 uPow3 = pow(u, 3);
	F32 vPow3 = pow(v, 3);
	F32 wPow3 = pow(w, 3);
	F32 uPow2 = pow(u, 2);
	F32 vPow2 = pow(v, 2);
	F32 wPow2 = pow(w, 2);

	Vec3 pos030 = IN_POS3(0);
	Vec3 pos003 = IN_POS3(1);
	Vec3 pos300 = IN_POS3(2);

	Vec3 pos = pos300 * wPow3 + pos030 * uPow3 + pos003 * vPow3 + pnPatch.pos210 * 3.0 * wPow2 * u
			   + pnPatch.pos120 * 3.0 * w * uPow2 + pnPatch.pos201 * 3.0 * wPow2 * v + pnPatch.pos021 * 3.0 * uPow2 * v
			   + pnPatch.pos102 * 3.0 * w * vPow2 + pnPatch.pos012 * 3.0 * u * vPow2 + pnPatch.pos111 * 6.0 * w * u * v;

	gl_Position = mvp * Vec4(pos, 1.0);
}

#define tessellatePhongPositionNormalTangentTexCoord_DEFINED
void tessellatePhongPositionNormalTangentTexCoord(in Mat4 mvp, in Mat3 normalMat)
{
#if PASS == COLOR
	outNormal = normalize(normalMat * INTERPOLATE(inNormal));
	outTangent = INTERPOLATE(inTangent);
	outTangent.xyz = normalize(normalMat * outTangent.xyz);
#endif

	outTexCoord = INTERPOLATE(inTexCoord);

	// interpolated position
	Vec3 inpos[3] = Vec3[](IN_POS3(0), IN_POS3(1), IN_POS3(2));
	Vec3 barPos = INTERPOLATE(inpos);

	// build terms
	Vec3 termIJ = Vec3(phongPatch.terms[0][0], phongPatch.terms[1][0], phongPatch.terms[2][0]);
	Vec3 termJK = Vec3(phongPatch.terms[0][1], phongPatch.terms[1][1], phongPatch.terms[2][1]);
	Vec3 termIK = Vec3(phongPatch.terms[0][2], phongPatch.terms[1][2], phongPatch.terms[2][2]);

	Vec3 tc2 = gl_TessCoord * gl_TessCoord;

	// phong tesselated pos
	Vec3 phongPos = tc2[0] * inpos[0] + tc2[1] * inpos[1] + tc2[2] * inpos[2]
					+ gl_TessCoord[0] * gl_TessCoord[1] * termIJ + gl_TessCoord[1] * gl_TessCoord[2] * termJK
					+ gl_TessCoord[2] * gl_TessCoord[0] * termIK;

	F32 tessAlpha = 1.0;
	Vec3 finalPos = (1.0 - tessAlpha) * barPos + tessAlpha * phongPos;
	gl_Position = mvp * Vec4(finalPos, 1.0);
}

#define tessellateDispMapPositionNormalTangentTexCoord_DEFINED
void tessellateDispMapPositionNormalTangentTexCoord(in Mat4 mvp, in Mat3 normalMat, in sampler2D dispMap)
{
	Vec3 norm = INTERPOLATE(inNormal);
#if PASS == COLOR
	outNormal = normalize(normalMat * norm);
	outTangent = INTERPOLATE(inTangent);
	outTangent.xyz = normalize(normalMat * outTangent.xyz);
#endif

	outTexCoord = INTERPOLATE(inTexCoord);

	F32 height = texture(dispMap, outTexCoord).r;
	height = height * 0.7 - 0.35;

	Vec3 inpos[3] = Vec3[](IN_POS3(0), IN_POS3(1), IN_POS3(2));
	Vec3 pos = INTERPOLATE(inpos) + norm * height;
	gl_Position = mvp * Vec4(pos, 1.0);
}
