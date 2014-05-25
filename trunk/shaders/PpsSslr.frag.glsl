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
	vec3 pp1 = project(p1);

	vec2 dir = pp1.xy - pp0;
	vec2 path = dir * 0.5; // (pp1.xy/2+1/2)-(pp0.xy/2+1/2)
	path *= vec2(float(WIDTH), float(HEIGHT));
	path = abs(path);
	float steps = max(path.x, path.y);

	// XXX
	float len = length(dir);
	float stepInc =  len / steps;
	dir /= len;

	steps = min(steps, 300.0);

	for(float i = 10.0; i < steps; i += 1.5)
	{
		vec2 ndc = pp0 + dir * (i * stepInc);

		vec2 comp = abs(ndc);
		if(comp.x > 1.0 || comp.y > 1.0)
		{
			//outColor = vec3(1, 0.0, 1);
			return;
		}

		vec3 a;
		a.z = -1.0;
		a.xy = ndc * uProjectionParams.xy * a.z;
		a = normalize(a);

		vec3 c0 = cross(a, r);
		vec3 c1 = cross(p0, r);
		float k = c1.x / c0.x;

		vec3 intersection = a * k;

		vec2 texCoord = ndc * 0.5 + 0.5;
		float depth = readZ(texCoord);

		float diffDepth = depth - intersection.z;

		if(diffDepth > 0.0)
		{
			if(diffDepth > 0.3)
			{
				outColor = vec3(0.01);
				return;
			}

			float factor = 1.0 - length(ndc.xy);
			factor *= 1.0 - length(pp0.xy);

			outColor = textureRt(uIsRt, texCoord).rgb * (factor * 1.1 /* specColor*/);

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
