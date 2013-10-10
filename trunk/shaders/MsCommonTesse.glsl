layout(triangles, equal_spacing, ccw) in;

// Varyings in
#if 0
in highp vec3 tcPosition[];
in highp vec2 tcTexCoords[];
#if PASS_COLOR
in mediump vec3 tcNormal[];
in mediump vec4 tcTangent[];
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

in patch OutPatch tcPatch;

// Varyings out
out highp vec2 teTexCoords;
#if PASS_COLOR
out mediump vec3 teNormal;
out mediump vec4 teTangent;
#endif

#define INTERPOLATE(x_) (x_[0] * gl_TessCoord.x + x_[1] * gl_TessCoord.y + x_[2] * gl_TessCoord.z)

// Smooth tessellation
#define subdivPositionNormalTangentTexCoord_DEFINED
void subdivPositionNormalTangentTexCoord(in mat4 mvp, in mat3 normalMat)
{
#if PASS_COLOR
	teNormal = normalize(normalMat * INTERPOLATE(tcPatch.normal));
	teTangent = INTERPOLATE(tcPatch.tangent);
	teTangent.xyz = normalize(normalMat * teTangent.xyz);
#endif

	teTexCoords = INTERPOLATE(tcPatch.texCoord);

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
		tcPatch.pos300 * wPow3
		+ tcPatch.pos030 * uPow3
		+ tcPatch.pos003 * vPow3
		+ tcPatch.pos210 * 3.0 * wPow2 * u 
		+ tcPatch.pos120 * 3.0 * w * uPow2 
		+ tcPatch.pos201 * 3.0 * wPow2 * v 
		+ tcPatch.pos021 * 3.0 * uPow2 * v 
		+ tcPatch.pos102 * 3.0 * w * vPow2 
		+ tcPatch.pos012 * 3.0 * u * vPow2 
		+ tcPatch.pos111 * 6.0 * w * u * v;

	gl_Position = mvp * vec4(pos, 1.0);
}
