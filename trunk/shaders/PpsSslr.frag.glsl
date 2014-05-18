// SSLR fragment shader
#pragma anki type frag
#pragma anki include "shaders/Common.glsl"
#pragma anki include "shaders/LinearDepth.glsl"
#pragma anki include "shaders/Pack.glsl"

layout(location = 0) in vec2 inTexCoords;

layout(location = 0) out vec3 outColor;

layout(std140, binding = 0) readonly buffer bCommon
{
	vec4 uProjectionParams;

	/// The projection matrix
	mat4 uProjectionMatrix;
};

layout(binding = 0) uniform sampler2D uIsRt;
layout(binding = 1) uniform sampler2D uMsDepthRt;
layout(binding = 2) uniform sampler2D uMsRt;

// Returns the Z of the position in view space
float readZ(in vec2 uv)
{
	float depth = textureRt(uMsDepthRt, uv).r;
	float z = uProjectionParams.z / (uProjectionParams.w + depth);
	return z;
}

// Read position in view space
vec3 readPosition(in vec2 uv)
{
	float depth = textureRt(uMsDepthRt, uv).r;

	vec3 fragPosVspace;
	fragPosVspace.z = readZ(uv);
	
	fragPosVspace.xy = 
		(2.0 * uv - 1.0) * uProjectionParams.xy * fragPosVspace.z;

	return fragPosVspace;
}

vec3 project(vec3 p)
{
	vec4 a = uProjectionMatrix * vec4(p, 1.0);
	a.xyz /= a.w;
	return a.xyz;
}

void main()
{
	vec3 normal;
	float specColor;
	readNormalSpecularColorFromGBuffer(uMsRt, inTexCoords, normal, specColor);

	if(specColor < 0.5)
	{
		outColor = vec3(0.0);
		return;
	}

	vec3 posv = readPosition(inTexCoords);

	// Reflection direction
	vec3 rDir = reflect(normalize(posv), normal);

	/*vec3 startpos = posv;
	vec3 endpos = startpos + rDir * 1.0;

	vec3 startposproj = project(startpos);
	vec3 endposproj = project(endpos);

	outColor = textureRt(uIsRt, endposproj.xy * 0.5 + 0.5).rgb;
	outColor = vec3(0.0);*/

#if 1
	vec3 pos = posv;
	for(int i = 0; i < 500; i++)
	{
		pos += rDir * 0.1;

		vec4 posNdc = uProjectionMatrix * vec4(pos, 1.0);
		posNdc.xyz /= posNdc.w;

		if(posNdc.x > 1.0 || posNdc.x < -1.0
			|| posNdc.y > 1.0 || posNdc.y < -1.0)
		{
			break;
		}

		vec3 posClip = posNdc.xyz * 0.5 + 0.5;

		float depth = textureRt(uMsDepthRt, posClip.xy).r;

		float diffDepth = posClip.z - depth;

		if(diffDepth > 0.0)
		{
			if(diffDepth > 0.001)
			{
				break;
			}

			float factor = 1.0 - length(posNdc.xy);

			outColor = textureRt(uIsRt, posClip.xy).rgb * (factor * specColor);
			//outColor = vec3(diffDepth);
			return;
		}
	}

	//outColor = vec3(0.5, 0.0, 0.0);
	outColor = vec3(0.0);
#endif
}
