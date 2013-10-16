layout(triangles, equal_spacing, ccw) in;

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

struct CommonPatch
{
	vec3 positions[3];
	vec2 texCoord[3];
	vec3 normal[3];
#if PASS_COLOR
	vec4 tangent[3];
#endif
};

in patch PNPatch pnPatch;
in patch PhongPatch phongPatch;
in patch CommonPatch commonPatch;

// Varyings out
out highp vec2 teTexCoords;
#if PASS_COLOR
out mediump vec3 teNormal;
out mediump vec4 teTangent;
#endif

#define INTERPOLATE(x_) (x_[0] * gl_TessCoord.x + x_[1] * gl_TessCoord.y + x_[2] * gl_TessCoord.z)

// Smooth tessellation
#define tessellatePNPositionNormalTangentTexCoord_DEFINED
void tessellatePNPositionNormalTangentTexCoord(in mat4 mvp, in mat3 normalMat)
{
#if PASS_COLOR
	teNormal = normalize(normalMat * INTERPOLATE(commonPatch.normal));
	teTangent = INTERPOLATE(commonPatch.tangent);
	teTangent.xyz = normalize(normalMat * teTangent.xyz);
#endif

	teTexCoords = INTERPOLATE(commonPatch.texCoord);

	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;
	float w = gl_TessCoord.z;

	float uPow3 = pow(u, 3);
	float vPow3 = pow(v, 3);
	float wPow3 = pow(w, 3);
	float uPow2 = pow(u, 2);
	float vPow2 = pow(v, 2);
	float wPow2 = pow(w, 2);

	vec3 pos = 
		commonPatch.pos300 * wPow3
		+ commonPatch.pos030 * uPow3
		+ commonPatch.pos003 * vPow3
		+ pnPatch.pos210 * 3.0 * wPow2 * u 
		+ pnPatch.pos120 * 3.0 * w * uPow2 
		+ pnPatch.pos201 * 3.0 * wPow2 * v 
		+ pnPatch.pos021 * 3.0 * uPow2 * v 
		+ pnPatch.pos102 * 3.0 * w * vPow2 
		+ pnPatch.pos012 * 3.0 * u * vPow2 
		+ pnPatch.pos111 * 6.0 * w * u * v;

	gl_Position = mvp * vec4(pos, 1.0);
}

#define tessellatePhongPositionNormalTangentTexCoord_DEFINED
void tessellatePhongPositionNormalTangentTexCoord(
	in mat4 mvp, in mat3 normalMat)
{
#if PASS_COLOR
	teNormal = normalize(normalMat * INTERPOLATE(commonPatch.normal));
	teTangent = INTERPOLATE(commonPatch.tangent);
	teTangent.xyz = normalize(normalMat * teTangent.xyz);
#endif

	teTexCoords = INTERPOLATE(commonPatch.texCoord);

	// interpolated position
	vec3 barPos = INTERPOLATE(commonPatch.positions);

	// build terms
	vec3 termIJ = vec3(
		phongPatch.terms[0][0],
		phongPatch.terms[1][0],
		phongPatch.terms[2][0]);
	vec3 termJK = vec3(
		phongPatch.terms[0][1],
		phongPatch.terms[1][1],
		phongPatch.terms[2][1]);
	vec3 termIK = vec3(
		phongPatch.terms[0][2],
		phongPatch.terms[1][2],
		phongPatch.terms[2][2]);

	vec3 tc2 = gl_TessCoord * gl_TessCoord;

	// phong tesselated pos
	vec3 phongPos = 
		tc2[0] * commonPatch.positions[0]
		 + tc2[1] * commonPatch.positions[1]
		 + tc2[2] * commonPatch.positions[2]
		 + gl_TessCoord[0] * gl_TessCoord[1] * termIJ
		 + gl_TessCoord[1] * gl_TessCoord[2] * termJK
		 + gl_TessCoord[2] * gl_TessCoord[0] * termIK;

	float uTessAlpha = 1.0;
	vec3 finalPos = (1.0 - uTessAlpha) * barPos + uTessAlpha * phongPos;
	gl_Position = mvp * vec4(finalPos, 1.0);
}

#define tessellateDispMapPositionNormalTangentTexCoord_DEFINED
void tessellateDispMapPositionNormalTangentTexCoord(
	in mat4 mvp, in mat3 normalMat, in sampler2D dispMap)
{
	vec3 norm = INTERPOLATE(commonPatch.normal);
#if PASS_COLOR
	teNormal = normalize(normalMat * norm);
	teTangent = INTERPOLATE(commonPatch.tangent);
	teTangent.xyz = normalize(normalMat * teTangent.xyz);
#endif

	teTexCoords = INTERPOLATE(commonPatch.texCoord);

	float height = texture(dispMap, teTexCoords).r;
	height *= 0.1;

	vec3 pos = INTERPOLATE(commonPatch.positions) + norm * height;
	gl_Position = mvp * vec4(pos, 1.0);
}


