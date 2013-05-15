/// @file
/// Generic shader program for Gaussian blur inspired by Daniel Rakos'
/// article
/// http://rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/
///
/// Switches: VPASS or HPASS, COL_RGBA or COL_RGB or COL_R
/// Also must define IMG_DIMENSION
///
/// This is an optimized version. See the clean one at r213

#pragma anki start vertexShader

layout(location = 0) in vec2 position;

out vec2 vTexCoords;

void main()
{
	vTexCoords = position;
	gl_Position = vec4(position * 2.0 - 1.0, 0.0, 1.0);
}

#pragma anki start fragmentShader

precision mediump float;

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

uniform mediump sampler2D img; ///< Input FAI

in vec2 vTexCoords;

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

// Weights
const float first_weight = 0.2270270270;
const float weights[4] = float[](
	0.3162162162, 0.0702702703, 
	0.3162162162, 0.0702702703);

// Calc the kernel
#if defined(VPASS)
#	define BLURRING_OFFSET_X(val) (float(val) / float(IMG_DIMENSION))
#	define BLURRING_OFFSET_Y(val) 0.0
#else
#	define BLURRING_OFFSET_X(val) 0.0
#	define BLURRING_OFFSET_Y(val) (float(val) / float(IMG_DIMENSION))
#endif

#define BLURRING_OFFSET(v) vec2(BLURRING_OFFSET_X(v), BLURRING_OFFSET_Y(v))

const vec2 kernel[4] = vec2[](
	BLURRING_OFFSET(1.3846153846),
	BLURRING_OFFSET(3.2307692308),
	BLURRING_OFFSET(-1.3846153846),
	BLURRING_OFFSET(-3.2307692308));

// Output
layout(location = 0) out COL_TYPE fFragColor;

void main()
{
	// the center (0,0) pixel
	fFragColor = texture(img, vTexCoords).TEX_FETCH * first_weight;

	// side pixels
	for(int i = 0; i < 4; i++)
	{
		fFragColor += 
			texture(img, vTexCoords + kernel[i]).TEX_FETCH * weights[i];
	}
}
