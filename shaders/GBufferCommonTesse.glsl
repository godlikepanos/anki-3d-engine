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
	vec3 pos021;
	vec3 pos012;
	vec3 pos102;
	vec3 pos201;
	vec3 pos210;
	vec3 pos120;
	vec3 pos111;
};

struct PhongPatch
{
	vec3 terms[3];
};

#if INSTANCE_ID_FRAGMENT_SHADER
struct CommonPatch
{
	uint instanceId;
};
#endif

in gl_PerVertex
{
	vec4 gl_Position;
}
gl_in[];

in patch PNPatch pnPatch;
in patch PhongPatch phongPatch;
#if INSTANCE_ID_FRAGMENT_SHADER
in patch CommonPatch commonPatch;
#endif

layout(location = 0) in vec2 inTexCoord[];
layout(location = 1) in vec3 inNormal[];
#if PASS == COLOR
layout(location = 2) in vec4 inTangent[];
#endif

//
// Output
//

out gl_PerVertex
{
	vec4 gl_Position;
};

layout(location = 0) out highp vec2 outTexCoord;
#if PASS == COLOR
layout(location = 1) out mediump vec3 outNormal;
layout(location = 2) out mediump vec4 outTangent;
#endif

#define INTERPOLATE(x_) (x_[0] * gl_TessCoord.x + x_[1] * gl_TessCoord.y + x_[2] * gl_TessCoord.z)

// Smooth tessellation
#define tessellatePNPositionNormalTangentTexCoord_DEFINED
void tessellatePNPositionNormalTangentTexCoord(in mat4 mvp, in mat3 normalMat)
{
#if PASS == COLOR
	outNormal = normalize(normalMat * INTERPOLATE(inNormal));
	outTangent = INTERPOLATE(inTangent);
	outTangent.xyz = normalize(normalMat * outTangent.xyz);
#endif

	outTexCoord = INTERPOLATE(inTexCoord);

	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;
	float w = gl_TessCoord.z;

	float uPow3 = pow(u, 3);
	float vPow3 = pow(v, 3);
	float wPow3 = pow(w, 3);
	float uPow2 = pow(u, 2);
	float vPow2 = pow(v, 2);
	float wPow2 = pow(w, 2);

	vec3 pos030 = IN_POS3(0);
	vec3 pos003 = IN_POS3(1);
	vec3 pos300 = IN_POS3(2);

	vec3 pos = pos300 * wPow3 + pos030 * uPow3 + pos003 * vPow3 + pnPatch.pos210 * 3.0 * wPow2 * u
			   + pnPatch.pos120 * 3.0 * w * uPow2 + pnPatch.pos201 * 3.0 * wPow2 * v + pnPatch.pos021 * 3.0 * uPow2 * v
			   + pnPatch.pos102 * 3.0 * w * vPow2 + pnPatch.pos012 * 3.0 * u * vPow2 + pnPatch.pos111 * 6.0 * w * u * v;

	gl_Position = mvp * vec4(pos, 1.0);
}

#define tessellatePhongPositionNormalTangentTexCoord_DEFINED
void tessellatePhongPositionNormalTangentTexCoord(in mat4 mvp, in mat3 normalMat)
{
#if PASS == COLOR
	outNormal = normalize(normalMat * INTERPOLATE(inNormal));
	outTangent = INTERPOLATE(inTangent);
	outTangent.xyz = normalize(normalMat * outTangent.xyz);
#endif

	outTexCoord = INTERPOLATE(inTexCoord);

	// interpolated position
	vec3 inpos[3] = vec3[](IN_POS3(0), IN_POS3(1), IN_POS3(2));
	vec3 barPos = INTERPOLATE(inpos);

	// build terms
	vec3 termIJ = vec3(phongPatch.terms[0][0], phongPatch.terms[1][0], phongPatch.terms[2][0]);
	vec3 termJK = vec3(phongPatch.terms[0][1], phongPatch.terms[1][1], phongPatch.terms[2][1]);
	vec3 termIK = vec3(phongPatch.terms[0][2], phongPatch.terms[1][2], phongPatch.terms[2][2]);

	vec3 tc2 = gl_TessCoord * gl_TessCoord;

	// phong tesselated pos
	vec3 phongPos = tc2[0] * inpos[0] + tc2[1] * inpos[1] + tc2[2] * inpos[2]
					+ gl_TessCoord[0] * gl_TessCoord[1] * termIJ + gl_TessCoord[1] * gl_TessCoord[2] * termJK
					+ gl_TessCoord[2] * gl_TessCoord[0] * termIK;

	float tessAlpha = 1.0;
	vec3 finalPos = (1.0 - tessAlpha) * barPos + tessAlpha * phongPos;
	gl_Position = mvp * vec4(finalPos, 1.0);
}

#define tessellateDispMapPositionNormalTangentTexCoord_DEFINED
void tessellateDispMapPositionNormalTangentTexCoord(in mat4 mvp, in mat3 normalMat, in sampler2D dispMap)
{
	vec3 norm = INTERPOLATE(inNormal);
#if PASS == COLOR
	outNormal = normalize(normalMat * norm);
	outTangent = INTERPOLATE(inTangent);
	outTangent.xyz = normalize(normalMat * outTangent.xyz);
#endif

	outTexCoord = INTERPOLATE(inTexCoord);

	float height = texture(dispMap, outTexCoord).r;
	height = height * 0.7 - 0.35;

	vec3 inpos[3] = vec3[](IN_POS3(0), IN_POS3(1), IN_POS3(2));
	vec3 pos = INTERPOLATE(inpos) + norm * height;
	gl_Position = mvp * vec4(pos, 1.0);
}
