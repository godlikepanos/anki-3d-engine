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

/// The offset of side pixels. Its actually a 2D array
const vec2 BLURRING_OFFSET = vec2(1.3846153846, 3.2307692308); 

void main()
{
	vTexCoords = position;
	gl_Position = vec4(position * 2.0 - 1.0, 0.0, 1.0);
}

#pragma anki start fragmentShader

// Preprocessor switches sanity checks
#if !defined(VPASS) && !defined(HPASS)
#	error "See file"
#endif

#if !(defined(COL_RGBA) || defined(COL_RGB) || defined(COL_R))
#	error "See file"
#endif

uniform sampler2D img; ///< Input FAI
uniform float blurringDist = 0.0;

in vec2 vTexCoords;
in vec2 vOffsets;

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

const float BLURRING_OFFSET[2] = float[](1.3846153846 / IMG_DIMENSION, 
	3.2307692308 / IMG_DIMENSION); 

layout(location = 0) out COL_TYPE fFragColor;

const float FIRST_WEIGHT = 0.2255859375;
const float WEIGHTS[2] = float[](0.314208984375, 0.06982421875);

void main()
{
	// the center (0,0) pixel
	COL_TYPE col = texture2D(img, vTexCoords).TEX_FETCH * FIRST_WEIGHT;

	// side pixels
	for(int i = 0; i < 2; i++)
	{
#if defined(HPASS)
		vec2 tc = vec2(vTexCoords.x + blurringDist + BLURRING_OFFSET[i], 
			vTexCoords.y);
		col += texture2D(img, tc).TEX_FETCH * WEIGHTS[i];

		tc.x = vTexCoords.x - blurringDist - BLURRING_OFFSET[i];
		col += texture2D(img, tc).TEX_FETCH * WEIGHTS[i];
#elif defined(VPASS)
		vec2 tc = vec2(vTexCoords.x, 
			vTexCoords.y + blurringDist + BLURRING_OFFSET[i]);
		col += texture2D(img, tc).TEX_FETCH * WEIGHTS[i];

		tc.y = vTexCoords.y - blurringDist - BLURRING_OFFSET[i];
		col += texture2D(img, tc).TEX_FETCH * WEIGHTS[i];
#endif
	}

	fFragColor = col;
}
