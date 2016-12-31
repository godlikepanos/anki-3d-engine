// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// SSLR functions and data
#include "shaders/Common.glsl"
#include "shaders/Functions.glsl"
#include "shaders/Pack.glsl"

const float ONE = 0.9;

// Returns the Z of the position in view space
float readZ(in vec2 uv)
{
	float depth = textureLod(u_depthRt, uv, 1.0).r;
	float z = u_projectionParams.z / (u_projectionParams.w + depth);
	return z;
}

vec2 projectXy(in vec3 p)
{
	vec4 a = u_projectionMat * vec4(p, 1.0);
	return a.xy / a.w;
}

vec3 doSslr(in vec3 r, in vec3 posVSpace, in vec2 uv, out float contribution)
{
	contribution = 0.5;
	return vec3(1.0, 0.0, 1.0);
#if 0
	vec3 color = vec3(0.0);
	vec3 p0 = posVSpace;
	contribution = 1.0;

	// Let p1 be the intersection of p0+r to the near plane, then
	// p1 = p0 + t*r or
	// p1.x = p0.x + t*r.x (1)
	// p1.y = p0.y + t*r.y (2) and
	// p1.z = p0.z + t*r.z (3)
	// p1.z is known to be something ~0.0 so if we solve (3) t becomes:
	float t = -p0.z / (r.z + 0.0000001);
	vec3 p1 = p0 + r * t;

	vec2 pp0 = uv * 2.0 - 1.0;
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

	for(float i = 0.0; i < steps; i += 1.0)
	{
		vec2 ndc = pp0 + dir * (i * stepInc);

		// Check if it's out of the view
		vec2 comp = abs(ndc);
		if(comp.x > ONE || comp.y > ONE)
		{
			//color = vec3(1, 0.0, 1);
			return color;
		}

		// 'a' is ray that passes through the eye and into ndc
		vec3 a;
		a.z = -1.0;
		a.xy = ndc * u_projectionParams.xy * a.z; // Unproject
		a = normalize(a);

		// Compute the intersection between 'a' (before normalization) and r 'k' is the value to multiply to 'a' to get 
		// the intersection
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
		float depth = readZ(u_depthRt, texCoord);

		float diffDepth = depth - intersectionZ;

		if(diffDepth > 0.0)
		{
			if(diffDepth > 0.7)
			{
				return;
			}

			float factor = sin(length(ndc) * PI);
			factor *= 1.0 - length(pp0);
			//factor *= specColor;

			color = textureLod(u_isRt, texCoord, 0.0).rgb * factor;

			//color = vec3(1.0, 0.0, 1.0);
			//color = vec3(1.0 - abs(pp0.xy), 0.0);
			return color;
		}
	}

	return color;
#endif
}
