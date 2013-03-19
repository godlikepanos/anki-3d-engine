layout(vertices = 3) out;

// Varyings in
in vec3 vPosition[];
in vec2 vTexCoords;
in vec3 vNormal[];
in vec3 vTangent[];
in float vTangentW[];

// Varyings out
out vec3 tcPosition[];
out vec2 tcTexCoords;
out vec3 tcNormal[];
out vec3 tcTangent[];
out float tcTangentW[];

#define tessellate_DEFINED
void tessellate(in float tessLevelInner, in float tessLevelOuter)
{
	if(gl_InvocationID == 0) 
	{
		gl_TessLevelInner[0] = tessLevelInner;
		gl_TessLevelOuter[0] = tessLevelOuter;
		gl_TessLevelOuter[1] = tessLevelOuter;
		gl_TessLevelOuter[2] = tessLevelOuter;
	}
}

#define tessellatePosition_DEFINED
void tessellatePosition()
{
	tcPosition[gl_InvocationID] = vPosition[gl_InvocationID];
}

#define tessellateTexCoords_DEFINED
void tessellateTexCoords()
{
	tcTexCoords[gl_InvocationID] = vTexCoords[gl_InvocationID];
}

#define tessellatieNormalTangent_DEFINED
void tessellatieNormalTangent()
{
	tcNormal[gl_InvocationID] = vNormal[gl_InvocationID];
	tcTangent[gl_InvocationID] = vTangent[gl_InvocationID];
	tcTangentW[gl_InvocationID] = vTangentW[gl_InvocationID];
}
