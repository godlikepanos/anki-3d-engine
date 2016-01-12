// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma anki type frag
#pragma anki include "shaders/Common.glsl"

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec3 out_color;

layout(TEX_BINDING(0, 0)) uniform sampler2D u_depthTexHigh;
layout(TEX_BINDING(0, 1)) uniform sampler2D u_depthTexLow;
layout(TEX_BINDING(0, 2)) uniform sampler2D u_colorTexNearest;
layout(TEX_BINDING(0, 3)) uniform sampler2D u_colorTexLinear;

const float DEPTH_THRESHOLD = 1.0 / 1000.0;

const vec2 COLOR_TEX_TEXEL_SIZE = 1.0 / vec2(TEXTURE_WIDTH, TEXTURE_HEIGHT);

void main()
{
	// Gather the depths around the UV
	vec4 depths = textureGather(u_depthTexLow, in_uv, 0);

	// Get the depth of the current fragment
	float depth = texture(u_depthTexHigh, in_uv).r;

	// Check for depth discontinuites. Use textureGrad because it's undefined
	// if you are sampling mipmaped in a conditional branch. See
	// http://teknicool.tumblr.com/post/77263472964/glsl-dynamic-branching-and-texture-samplers
	vec2 texDx = dFdx(in_uv);
	vec2 texDy = dFdy(in_uv);
	vec4 diffs = abs(depths - vec4(depth));
	if(all(lessThan(diffs, vec4(DEPTH_THRESHOLD))))
	{
		// No major discontinuites, sample with bilinear
		out_color = textureGrad(u_colorTexLinear, in_uv, texDx, texDy).rgb;
	}
	else
	{
		// Some discontinuites, need to do some work
		// Note: Texture gather values are placed like this:
		// xy
		// wz
		// and w is the current texel

		float smallestDiff = diffs.w;
		vec2 newUv = in_uv;

		if(diffs.z < smallestDiff)
		{
			smallestDiff = diffs.z;
			newUv.x += COLOR_TEX_TEXEL_SIZE.x;
		}

		if(diffs.y < smallestDiff)
		{
			smallestDiff = diffs.y;
			newUv = in_uv + COLOR_TEX_TEXEL_SIZE;
		}

		if(diffs.x < smallestDiff)
		{
			newUv = in_uv + vec2(0.0, COLOR_TEX_TEXEL_SIZE.y);
		}

		out_color = textureGrad(u_colorTexNearest, newUv, texDx, texDy).rgb;
	}

	/*vec2 texDx = dFdx(in_uv);
	vec2 texDy = dFdy(in_uv);
	out_color = textureGrad(u_colorTexLinear, in_uv, texDx, texDy).rgb;*/
}


