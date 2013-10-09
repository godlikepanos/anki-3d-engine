layout(triangles, equal_spacing, ccw) in;

// Varyings in
in highp vec3 tcPosition[];
in highp vec2 tcTexCoords[];
#if PASS_COLOR
in mediump vec3 tcNormal[];
in mediump vec4 tcTangent[];
#endif

// Varyings out
out highp vec3 tePosition;
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
	teNormal = normalize(normalMat * INTERPOLATE(tcNormal));
	teTangent = INTERPOLATE(tcTangent);
	teTangent.xyz = normalize(normalMat * teTangent.xyz);
#endif

	teTexCoords = INTERPOLATE(tcTexCoords);

	gl_Position = mvp * vec4(INTERPOLATE(tcPosition), 1.0);
}
