// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "shaders/Pack.glsl"

// Common
layout(TEX_BINDING(0, 0)) uniform sampler2D u_depthRt;
layout(TEX_BINDING(0, 1)) uniform sampler2D u_msRt0;
layout(TEX_BINDING(0, 2)) uniform sampler2D u_msRt1;
layout(TEX_BINDING(0, 3)) uniform sampler2D u_msRt2;

layout(std140, UBO_BINDING(0, 0)) uniform u0_
{
	vec4 u_projectionParams;
	mat4 u_projectionMat;
};

// SSLR
#if SSLR_ENABLED
layout(TEX_BINDING(0, 4)) uniform sampler2D u_isRt;
#include "shaders/Sslr.glsl"
#endif

// IR
#if IR_ENABLED
#define IMAGE_REFLECTIONS_SET 0
#define IMAGE_REFLECTIONS_FIRST_SS_BINDING 0
#define IMAGE_REFLECTIONS_TEX_BINDING 5
#include "shaders/ImageReflections.glsl"
#undef IMAGE_REFLECTIONS_SET
#undef IMAGE_REFLECTIONS_FIRST_SS_BINDING
#endif

// In/out
layout(location = 0) in vec2 in_texCoord;
layout(location = 0) out vec3 out_indirectColor;

void main()
{
	//
	// Decode the G-buffer
	//
	float depth = textureLod(u_depthRt, in_texCoord, 0.0).r;
	vec3 posVSpace;
	posVSpace.z = u_projectionParams.z / (u_projectionParams.w + depth);
	posVSpace.xy =
		(2.0 * in_texCoord - 1.0) * u_projectionParams.xy * posVSpace.z;

	GbufferInfo gbuffer;
	readGBuffer(u_msRt0, u_msRt1, u_msRt2, in_texCoord, 0.0, gbuffer);

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

	vec3 specIndirect, diffIndirect;
	readIndirect(
		posVSpace, r, gbuffer.normal, reflLod, specIndirect, diffIndirect);

	diffIndirect *= gbuffer.diffuse;
#endif

	// Finalize the indirect specular
	float ndotv = dot(gbuffer.normal, -eye);
	vec2 envBRDF = texture(u_integrationLut, vec2(gbuffer.roughness, ndotv)).xy;
	specIndirect = specIndirect * (gbuffer.specular * envBRDF.x + envBRDF.y);

	// Finalize
	out_indirectColor = diffIndirect + specIndirect;
	gl_FragDepth = depth;
}
