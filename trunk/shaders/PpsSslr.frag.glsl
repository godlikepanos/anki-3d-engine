// SSLR fragment shader
#pragma anki type frag
#pragma anki include "shaders/Common.glsl"
#pragma anki include "shaders/LinearDepth.glsl"

layout(location = 0) in vec2 inTexCoords;

layout(location = 0) out vec3 outColor;

layout(std140, binding = 0) readonly buffer bCommon
{
	/// Packs:
	/// - x: uZNear. For the calculation of frag pos in view space
	/// - zw: Planes. For the calculation of frag pos in view space
	vec4 uNearPlanesComp;

	/// For the calculation of frag pos in view space. The xy is the 
	/// uLimitsOfNearPlane and the zw is an optimization see PpsSsao.glsl and 
	/// r403 for the clean one
	vec4 uLimitsOfNearPlaneComp;

	/// The projection matrix
	mat4 uProjectionMatrix;
};

#define uZNear uNearPlanesComp.x
#define uZFar uNearPlanesComp.y
#define uPlanes uNearPlanesComp.zw
#define uLimitsOfNearPlane uLimitsOfNearPlaneComp.xy
#define uLimitsOfNearPlane2 uLimitsOfNearPlaneComp.zw

layout(binding = 0) uniform sampler2D uIsRt;
layout(binding = 1) uniform sampler2D uMsDepthRt;
layout(binding = 2) uniform sampler2D uMsRt;

// Get normal
vec3 readNormal(in vec2 uv)
{
	vec3 normal;
	readNormalFromGBuffer(uMsRt, uv, normal);
	return normal;
}

// Get position in view space
vec3 readPosition(in vec2 uv)
{
	float depth = textureRt(uMsDepthRt, uv).r;

	vec3 fragPosVspace;
	fragPosVspace.z = -uPlanes.y / (uPlanes.x + depth);
	
	fragPosVspace.xy = (uv * uLimitsOfNearPlane2) - uLimitsOfNearPlane;
	
	float sc = -fragPosVspace.z;
	fragPosVspace.xy *= sc;

	return fragPosVspace;
}

float readZ(in vec2 uv)
{
	float depth = textureRt(uMsDepthRt, uv).r;
	float z = -uPlanes.y / (uPlanes.x + depth);
	return z;
}

void main()
{
	vec3 normal = readNormal(inTexCoords);
	vec3 pos = readPosition(inTexCoords);

	// Reflection direction
	vec3 rDir = normalize(reflect(pos, normal));

	vec3 sample = pos;
	for(int i = 0; i < 20; i++)
	{
		sample += rDir;

		vec4 sampleNdc = uProjectionMatrix * vec4(sample, 1.0);
		sampleNdc.xy /= sampleNdc.w;
		sampleNdc.xy = sampleNdc.xy * 0.5 + 0.5;

		depth = readZ(sampleNdc.xy);

		float diffDepth = sampleNdc.z - depth;

		if(dDepth < 0.0)
		{
			outColor = texture(uIsRt, sampleNdc.xy);
			return;
		}
	}

	outColor = vec3(0.0);
}
