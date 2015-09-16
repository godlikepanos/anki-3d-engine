// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Contains functions for light calculations

#ifndef ANKI_SHADERS_LIGHT_FUNCTIONS_GLSL
#define ANKI_SHADERS_LIGHT_FUNCTIONS_GLSL

#pragma anki include "shaders/Common.glsl"

const float ATTENUATION_BOOST = 0.05;
const float OMNI_LIGHT_FRUSTUM_NEAR_PLANE = 0.1 / 4.0;

//==============================================================================
/// Calculate the cluster split
uint calcClusterSplit(float zVspace)
{
	zVspace = -zVspace;
	float fk = sqrt(
		(zVspace - u_nearFarClustererDivisor.x) / u_nearFarClustererDivisor.z);
	uint k = uint(fk);
	return k;
}

//==============================================================================
float computeAttenuationFactor(float lightRadius, vec3 frag2Light)
{
	float fragLightDist = length(frag2Light);

	float att = (fragLightDist * lightRadius) + (1.0 + ATTENUATION_BOOST);
	att = max(0.0, att);
	att *= att;

	return att;
}

//==============================================================================
// Performs BRDF specular lighting
vec3 computeSpecularColorBrdf(
	vec3 v, // view dir
	vec3 l, // light dir
	vec3 n, // normal
	vec3 specCol,
	vec3 lightSpecCol,
	float a2, // rougness^2
	float nol) // N dot L
{
	vec3 h = normalize(l + v);

	// Fresnel (Schlick)
	float loh = max(EPSILON, dot(l, h));
	vec3 f = specCol + (1.0 - specCol) * pow((1.0 + EPSILON - loh), 5.0);
	//float f = specColor + (1.0 - specColor)
	//	* pow(2.0, (-5.55473 * loh - 6.98316) * loh);

	// NDF: GGX Trowbridge-Reitz
	float noh = max(EPSILON, dot(n, h));
	float d = a2 / (PI * pow(noh * noh * (a2 - 1.0) + 1.0, 2.0));

	// Visibility term: Geometric shadowing devided by BRDF denominator
	float nov = max(EPSILON, dot(n, v));
	float vv = nov + sqrt((nov - nov * a2) * nov + a2);
	float vl = nol + sqrt((nol - nol * a2) * nol + a2);
	float vis = 1.0 / (vv * vl);

	return f * (vis * d) * lightSpecCol;
}

//==============================================================================
vec3 computeDiffuseColor(vec3 diffCol, vec3 lightDiffCol)
{
	return diffCol * lightDiffCol;
}

//==============================================================================
float computeSpotFactor(
	vec3 l,
	float outerCos,
	float innerCos,
	vec3 spotDir)
{
	float costheta = -dot(l, spotDir);
	float spotFactor = smoothstep(outerCos, innerCos, costheta);
	return spotFactor;
}

//==============================================================================
float computeShadowFactorSpot(mat4 lightProjectionMat, vec3 fragPos,
	float layer)
{
	vec4 texCoords4 = lightProjectionMat * vec4(fragPos, 1.0);
	vec3 texCoords3 = texCoords4.xyz / texCoords4.w;

#if POISSON == 1
	const vec2 poissonDisk[4] = vec2[](
		vec2(-0.94201624, -0.39906216),
		vec2(0.94558609, -0.76890725),
		vec2(-0.094184101, -0.92938870),
		vec2(0.34495938, 0.29387760));

	float shadowFactor = 0.0;

	vec2 cordpart0 = vec2(layer, texCoords3.z);

	for(int i = 0; i < 4; i++)
	{
		vec2 cordpart1 = texCoords3.xy + poissonDisk[i] / (300.0);
		vec4 tcoord = vec4(cordpart1, cordpart0);

		shadowFactor += texture(u_spotMapArr, tcoord);
	}

	return shadowFactor / 4.0;
#else
	vec4 tcoord = vec4(texCoords3.x, texCoords3.y, layer, texCoords3.z);
	float shadowFactor = texture(u_spotMapArr, tcoord);

	return shadowFactor;
#endif
}

//==============================================================================
float computeShadowFactorOmni(vec3 frag2Light, float layer, float radius)
{
	vec3 dir = (u_viewMat * vec4(-frag2Light, 1.0)).xyz;
	vec3 dirabs = abs(dir);
	float dist = -max(dirabs.x, max(dirabs.y, dirabs.z));
	dir = normalize(dir);

	const float near = OMNI_LIGHT_FRUSTUM_NEAR_PLANE;
	const float far = radius;

	// Original code:
	// float g = near - far;
	// float z = (far + near) / g * dist + (2.0 * far * near) / g;
	// float w = -dist;
	// z /= w;
	// z = z * 0.5 + 0.5;
	// Optimized:
	float z = (far * (dist + near)) / (dist * (far - near));

	float shadowFactor = texture(u_omniMapArr, vec4(dir, layer), z).r;
	return shadowFactor;
}

#endif

