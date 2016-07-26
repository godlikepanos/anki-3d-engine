// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Packing.h"

layout(location = 0) vec3 in_posViewSpace;

layout(location = 0) vec3 out_color;

// Point light
struct PointLight
{
	vec4 posRadius; // xyz: Light pos in eye space. w: The -1/radius
	vec4 diffuseColorPad1; // xyz: diff color
	vec4 specularColorPad1; // xyz: spec color
};

// Spot light
struct SpotLight
{
	vec4 posRadius; // xyz: Light pos in eye space. w: The -1/radius
	vec4 diffuseColorOuterCos; // xyz: diff color, w: outer cosine of spot
	vec4 specularColorInnerCos; // xyz: spec color, w: inner cosine of spot
	vec4 lightDir;
}

layout(ANKI_TEX_BINDING(0, 0)) uniform sampler2D u_msRt0;
layout(ANKI_TEX_BINDING(0, 1)) uniform sampler2D u_msRt1;
layout(ANKI_TEX_BINDING(0, 2)) uniform sampler2D u_msRt2;

layout(ANKI_UBO_BINDING(0, 1)) uniform u1_
{
#if defined(POINT_LIGHT)
	PointLight u_light;
#elif defined(SPOT_LIGHT)
	SpotLight u_light;
#else
#error See file
#endif
};

#if defined(POINT_LIGHT)
#define u_ldiff u_light.diffuseColorPad1.xyz
#define u_lspec u_light.specularColorPad1.xyz
#else
#define u_ldiff u_light.diffuseColorOuterCos.xyz
#define u_lspec u_light.specularColorInnerCos.xyz
#endif

void main()
{
	// Read G-buffer
	vec2 uv = vec2(gl_FragCoord.xy) / vec2(RENDERING_WIDTH, RENDERING_HEIGHT);
	GbufferInfo gbuffer;
	readGBuffer(u_msRt0, u_msRt1, u_msRt2, uv, 0.0, gbuffer);
	float a2 = pow(roughness, 2.0);

	// Calculate the light color
	vec3 viewDir = normalize(-in_posViewSpace);
	vec3 frag2Light = u_light.posRadius.xyz - in_posViewSpace;
	vec3 l = normalize(frag2Light);
	float nol = max(0.0, dot(normal, l));

	vec3 specC = computeSpecularColorBrdf(
		viewDir, l, gbuffer.normal, gbuffer.specCol, u_lspec, a2, nol);

	vec3 diffC = computeDiffuseColor(gbuffer.diffuse, u_ldiff);

	float att = computeAttenuationFactor(u_light.posRadius.w, frag2Light);
	float lambert = nol;

#if defined(POINT_LIGHT)
	out_color = (specC + diffC) * (att * lambert);
#else
	float spot = computeSpotFactor(l,
		u_light.diffuseColorOuterCos.w,
		u_light.specularColorInnerCos.w,
		u_light.lightDir.xyz);

	out_color = (diffC + specC) * (att * spot * lambert);
#endif
}
