// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma anki type frag
#pragma anki include "shaders/Pack.glsl"

// Common
layout(TEX_BINDING(0, 0)) uniform sampler2D u_depthRt;
layout(TEX_BINDING(0, 1)) uniform sampler2D u_msRt2;

layout(std140, UBO_BINDING(0, 0)) uniform u0_
{
	vec4 u_projectionParams;
	mat4 u_projectionMat;
};

// IR
#if IR_ENABLED
#define IMAGE_REFLECTIONS_SET 0
#define IMAGE_REFLECTIONS_FIRST_SS_BINDING 0
#define IMAGE_REFLECTIONS_TEX_BINDING 4
#pragma anki include "shaders/ImageReflections.glsl"
#undef IMAGE_REFLECTIONS_SET
#undef IMAGE_REFLECTIONS_FIRST_SS_BINDING
#endif

// SSLR
#if SSLR_ENABLED
layout(TEX_BINDING(0, 3)) uniform sampler2D u_isRt;

#pragma anki include "shaders/Sslr.glsl"
#endif

// In/out
layout(location = 0) in vec2 in_texCoord;
layout(location = 0) out vec3 out_color;

void main()
{
	//
	// Decode the G-buffer
	//
	float depth = texture(u_depthRt, in_texCoord).r;
	vec3 posVSpace;
	posVSpace.z = u_projectionParams.z / (u_projectionParams.w + depth);
	posVSpace.xy =
		(2.0 * in_texCoord - 1.0) * u_projectionParams.xy * posVSpace.z;

	GbufferInfo gbuffer;
	readNormalRoughnessMetallicFromGBuffer(u_msRt2, in_texCoord, gbuffer);

	// Compute relflection vector
	vec3 eye = normalize(posVSpace);
	vec3 r = reflect(eye, gbuffer.normal);

	//
	// SSLR
	//
#if SSLR_ENABLED
	float sslrContribution;

	// Don't bother for very rough surfaces
	if(gbuffer.roughness > SSLR_START_ROUGHNESS)
	{
		sslrContribution = 1.0;
		out_color = vec3(1.0, 0.0, 1.0);
	}
	else
	{
		sslrContribution = 0.0;
	}
#else
	const float sslrContribution = 0.0;
#endif

	//
	// IR
	//
#if IR_ENABLED
	float reflLod = float(IR_MIPMAP_COUNT) * gbuffer.roughness;
	vec3 imgRefl = doImageReflections(posVSpace, r, reflLod);
	out_color = mix(imgRefl, out_color, sslrContribution);
#endif

	out_color *= gbuffer.metallic;
}
