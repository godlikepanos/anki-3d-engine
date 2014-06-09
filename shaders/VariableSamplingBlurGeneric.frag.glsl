// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

/// Defines: 
/// - VPASS or HPASS
/// - COL_RGBA or COL_RGB or COL_R
/// - SAMPLES is a number of 3 or 5 or 7 or 9
/// - BLURRING_DIST is optional and it's extra pixels to move the blurring

#pragma anki type frag

#define DEFAULT_FLOAT_PRECISION mediump
#pragma anki include "shaders/Common.glsl"

// Preprocessor switches sanity checks
#if !defined(VPASS) && !defined(HPASS)
#	error See file
#endif

#if !(defined(COL_RGBA) || defined(COL_RGB) || defined(COL_R))
#	error See file
#endif

#if !defined(IMG_DIMENSION)
#	error See file
#endif

#if !defined(SAMPLES)
#	error See file
#endif

// Input RT with different binding for less GL API calls
#if defined(VPASS)
layout(binding = 0) 
#else
layout(binding = 1) 
#endif
	uniform mediump sampler2D uTex; 

layout(location = 0) in vec2 inTexCoord;

#if !defined(BLURRING_DIST)
#	define BLURRING_DIST 0.0
#endif

// Determine color type
#if defined(COL_RGBA)
#	define COL_TYPE vec4
#elif defined(COL_RGB)
#	define COL_TYPE vec3
#elif defined(COL_R)
#	define COL_TYPE float
#endif

// Determine tex fetch
#if defined(COL_RGBA)
#	define TEX_FETCH rgba
#elif defined(COL_RGB)
#	define TEX_FETCH rgb
#elif defined(COL_R)
#	define TEX_FETCH r
#endif

// Calc the kernel. Use offsets of 3 to take advantage of bilinear filtering
#define BLURRING(val, sign_) ((float(val) * (float(BLURRING_DIST) + 1.0) / float(IMG_DIMENSION)) * float(sign_))

#if defined(VPASS)
#	define BLURRING_OFFSET_X(val, sign_) BLURRING(val, sign_)
#	define BLURRING_OFFSET_Y(val, sign_) 0.0
#else
#	define BLURRING_OFFSET_X(val, sign_) 0.0
#	define BLURRING_OFFSET_Y(val, sign_) BLURRING(val, sign_)
#endif

#define BLURRING_OFFSET(v, s) vec2(BLURRING_OFFSET_X(v, s), BLURRING_OFFSET_Y(v, s))

const vec2 kernel[SAMPLES - 1] = vec2[](
	BLURRING_OFFSET(1, -1),
	BLURRING_OFFSET(1, 1)
#if SAMPLES > 3
	,BLURRING_OFFSET(2, -1),
	BLURRING_OFFSET(2, 1)
#endif
#if SAMPLES > 5
	,BLURRING_OFFSET(3, -1),
	BLURRING_OFFSET(3, 1)
#endif
#if SAMPLES > 7
	,BLURRING_OFFSET(4, -1),
	BLURRING_OFFSET(4, 1)
#endif
#if SAMPLES > 9
	,BLURRING_OFFSET(5, -1),
	BLURRING_OFFSET(5, 1)
#endif
#if SAMPLES > 11
	,BLURRING_OFFSET(6, -1),
	BLURRING_OFFSET(6, 1)
#endif
#if SAMPLES > 13
	,BLURRING_OFFSET(7, -1),
	BLURRING_OFFSET(7, 1)
#endif
#if SAMPLES > 15
	,BLURRING_OFFSET(8, -1),
	BLURRING_OFFSET(8, 1)
#endif
#if SAMPLES > 17
	,BLURRING_OFFSET(9, -1),
	BLURRING_OFFSET(9, 1)
#endif
	);

layout(location = 0) out COL_TYPE outFragColor;

void main()
{
	// Get the first
	COL_TYPE col = textureRt(uTex, inTexCoord).TEX_FETCH;

	// Get the rest of the samples
	for(uint i = 0; i < SAMPLES - 1; i++)
	{
		col += textureRt(uTex, inTexCoord + kernel[i]).TEX_FETCH;
	}

	outFragColor = col / float(SAMPLES);
}
