// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Pack.glsl"
#include "shaders/LightFunctions.glsl"

layout(location = 0) out vec3 out_color;

// Point light
struct PointLight
{
	vec4 projectionParams;
	vec4 posRadius; // xyz: Light pos in eye space. w: The -1/radius
	vec4 diffuseColorPad1; // xyz: diff color
	vec4 specularColorPad1; // xyz: spec color
};

// Spot light
struct SpotLight
{
	vec4 projectionParams;
	vec4 posRadius; // xyz: Light pos in eye space. w: The -1/radius
	vec4 diffuseColorOuterCos; // xyz: diff color, w: outer cosine of spot
	vec4 specularColorInnerCos; // xyz: spec color, w: inner cosine of spot
	vec4 lightDirPad1;
};

layout(ANKI_TEX_BINDING(0, 0)) uniform sampler2D u_msRt0;
layout(ANKI_TEX_BINDING(0, 1)) uniform sampler2D u_msRt1;
layout(ANKI_TEX_BINDING(0, 2)) uniform sampler2D u_msRt2;
layout(ANKI_TEX_BINDING(0, 3)) uniform sampler2D u_msDepthRt;

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

vec3 readPosition(in vec2 uv)
{
	vec3 fragPosVspace;

	float depth = texture(u_msDepthRt, uv).r;
	fragPosVspace.z = u_light.projectionParams.z / (u_light.projectionParams.w + depth);

	fragPosVspace.xy = (2.0 * uv - 1.0) * u_light.projectionParams.xy * fragPosVspace.z;

	return fragPosVspace;
}

void main()
{
	// Read G-buffer
	vec2 uv = vec2(gl_FragCoord.xy) / vec2(RENDERING_WIDTH, RENDERING_HEIGHT);
	GbufferInfo gbuffer;
	readGBuffer(u_msRt0, u_msRt1, u_msRt2, uv, 0.0, gbuffer);
	float a2 = pow(gbuffer.roughness, 2.0);

	// Calculate the light color
	vec3 fragPos = readPosition(uv);
	vec3 viewDir = normalize(-fragPos);
	vec3 frag2Light = u_light.posRadius.xyz - fragPos;
	vec3 l = normalize(frag2Light);
	float nol = max(0.0, dot(gbuffer.normal, l));

	vec3 specC = computeSpecularColorBrdf(viewDir, l, gbuffer.normal, gbuffer.specular, u_lspec, a2, nol);

	vec3 diffC = computeDiffuseColor(gbuffer.diffuse, u_ldiff);

	float att = computeAttenuationFactor(u_light.posRadius.w, frag2Light);
	float lambert = nol;

#if defined(POINT_LIGHT)
	out_color = (specC + diffC) * (att * max(lambert, gbuffer.subsurface));
#else
	float spot =
		computeSpotFactor(l, u_light.diffuseColorOuterCos.w, u_light.specularColorInnerCos.w, u_light.lightDirPad1.xyz);

	out_color = (diffC + specC) * (att * spot * lambert);
#endif
}
