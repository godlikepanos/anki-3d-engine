// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma anki type frag
#pragma anki include "shaders/Pack.glsl"

// Common
layout(TEX_BINDING(0, 0)) uniform sampler2D u_depthRt;
layout(TEX_BINDING(0, 1)) uniform sampler2D u_msRt1;
layout(TEX_BINDING(0, 2)) uniform sampler2D u_msRt2;

layout(std140, UBO_BINDING(0, 0)) uniform _blk0
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

#if 0
	#if IR == 1
	{
		float reflLod = float(IR_MIPMAP_COUNT) * roughness;
		vec3 refl = readReflection(clusterIndex, fragPos, normal, reflLod);
		out_color += refl * (1.0 - roughness);
	}
#endif

// Don't bother for very rough surfaces
	if(roughness > SSLR_START_ROUGHNESS)
	{
		contribution = 0.0;
		return vec3(0.0);
	}

	// Decode the G-buffer
	float depth = textureRt(u_msDepthRt, in_texCoord).r;
	vec3 posVSpace;
	posVSpace.z = u_projectionParams.z / (u_projectionParams.w + depth);
	posVSpace.xy =
		(2.0 * in_texCoord - 1.0) * u_projectionParams.xy * posVSpace.z;

	float roughness;
	readRoughnessFromGBuffer(u_rt1, in_texCoord, roughness);

	vec3 normal;
	readNormalFromGBuffer(u_rt2, in_texCoord, normal);

	// First the SSLR
#if SSLR_ENABLED

#else
#endif

#endif

	out_color = vec3(0.0, 0.0, 1.0);
}
