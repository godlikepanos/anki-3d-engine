// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// SSLR fragment shader
#pragma anki type frag
#pragma anki include "shaders/Common.glsl"
#pragma anki include "shaders/LinearDepth.glsl"
#pragma anki include "shaders/Pack.glsl"

const float ONE = 0.9;

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
	return a.xyz / a.w;
}

vec2 projectXy(vec3 p)
{
	vec4 a = uProjectionMatrix * vec4(p, 1.0);
	return a.xy / a.w;
}

void main()
{
	vec3 normal;
	float specColor;
	readNormalSpecularColorFromGBuffer(uMsRt, inTexCoords, normal, specColor);

	//outColor = vec3(0.5, 0.0, 0.0);
	outColor = vec3(0.0);

	if(specColor < 0.5)
	{
		return;
	}

	vec3 p0 = readPosition(inTexCoords);

	// Reflection direction
	vec3 eye = normalize(p0);
	vec3 r = reflect(eye, normal);

#if 1
	// Let p1 be the intersection of p0+r to the near plane, then
	// p1 = p0 + t*r or p1.x = p0.x + t*r.x, p1.y = p0.y + t*r.y and
	// p1.z = p0.z + t*r.z (3)
	// p1.z is known to ~0.0 so if we solve (3) t becomes:
	float t = -p0.z / (r.z + 0.0000001);
	vec3 p1 = p0 + r * t;

	vec2 pp0 = inTexCoords * 2.0 - 1.0;
	vec2 pp1 = projectXy(p1);

	// Calculate the ray from p0 to p1 in 2D space and get the number of
	// steps
	vec2 dir = pp1 - pp0;
	vec2 path = dir * 0.5; // (pp1/2+1/2)-(pp0.xy/2+1/2)
	path *= vec2(float(WIDTH), float(HEIGHT));
	path = abs(path);
	float steps = max(path.x, path.y);

	// Calculate the step increase
	float len = length(dir);
	float stepInc =  len / steps;
	dir /= len; // Normalize dir at last

	steps = min(steps, 300.0);

	for(float i = 10.0; i < steps; i += 1.5)
	{
		vec2 ndc = pp0 + dir * (i * stepInc);

		// Check if it's out of the view
		vec2 comp = abs(ndc);
		if(comp.x > ONE || comp.y > ONE)
		{
			//outColor = vec3(1, 0.0, 1);
			return;
		}

		// 'a' is ray that passes through the eye and into ndc
		vec3 a;
		a.z = -1.0;
		a.xy = ndc * uProjectionParams.xy * a.z; // Unproject
		a = normalize(a);

		// Compute the intersection between 'a' (before normalization) and p0-p1 
		// c0 = cross(a, r);
		// c1 = cross(p0, r);
		// k = c1.x / c0.x; and the optimized:
		vec2 tmpv2 = a.yz * r.zy;
		float c0x = tmpv2.x - tmpv2.y;
		tmpv2 = p0.yz * r.zy;
		float c1x = tmpv2.x - tmpv2.y;
		float k = c1x / c0x;

		float intersectionZ = a.z * k; // intersectionXYZ = a * k;

		vec2 texCoord = ndc * 0.5 + 0.5;
		float depth = readZ(texCoord);

		float diffDepth = depth - intersectionZ;

		if(diffDepth > 0.0)
		{
			if(diffDepth > 0.7)
			{
				return;
			}

			float factor = 1.0 - length(ndc);
			factor *= 1.0 - length(pp0);
			factor *= specColor;

			outColor = textureRt(uIsRt, texCoord).rgb * factor;

			//outColor = vec3(1.0 - abs(pp0.xy), 0.0);
			return;
		}
	}
#endif

#if 0
	vec3 pos = p0;
	for(int i = 0; i < 200; i++)
	{
		pos += r * 0.1;

		vec3 posNdc = project(pos);

		vec2 comp = abs(posNdc.xy);
		if(comp.x > 1.0 || comp.y > 1.0)
		{
			return;
		}

		vec3 texCoord = posNdc.xyz * 0.5 + 0.5;

		float depth = textureRt(uMsDepthRt, texCoord.xy).r;

		float diffDepth = texCoord.z - depth;

		if(diffDepth > 0.0)
		{
			if(diffDepth > 0.001)
			{
				break;
			}

			float factor = 1.0 - length(posNdc.xy);

			outColor = textureRt(uIsRt, texCoord.xy).rgb * (factor * specColor);
			//outColor = vec3(diffDepth);
			return;
		}
	}
#endif
}
