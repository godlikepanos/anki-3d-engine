/// Defines: 
/// - VPASS or HPASS
/// - COL_RGBA or COL_RGB or COL_R
/// - SAMPLES is a number of 2 or 4 or 6

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

#if !defined(SAMPLES)
#	error See file
#endif

uniform mediump sampler2D img; ///< Input FAI

in vec2 vTexCoords;

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

// Calc the kernel
#if defined(VPASS)
#	define BLURRING_OFFSET_X(val) (float(val) / float(IMG_DIMENSION))
#	define BLURRING_OFFSET_Y(val) 0.0
#else
#	define BLURRING_OFFSET_X(val) 0.0
#	define BLURRING_OFFSET_Y(val) (float(val) / float(IMG_DIMENSION))
#endif

#define BLURRING_OFFSET(v) vec2(BLURRING_OFFSET_X(v), BLURRING_OFFSET_Y(v))

#if SAMPLES == 2
const vec2 kernel[3] = vec2[](
	BLURRING_OFFSET(-1),
	BLURRING_OFFSET(0),
	BLURRING_OFFSET(1));
#elif SAMPLES == 4
const vec2 kernel[5] = vec2[](
	BLURRING_OFFSET(-2),
	BLURRING_OFFSET(-1),
	BLURRING_OFFSET(0),
	BLURRING_OFFSET(1),
	BLURRING_OFFSET(2));
#elif SAMPLES == 6
const vec2 kernel[7] = vec2[](
	BLURRING_OFFSET(-3),
	BLURRING_OFFSET(-2),
	BLURRING_OFFSET(-1),
	BLURRING_OFFSET(0),
	BLURRING_OFFSET(1),
	BLURRING_OFFSET(2),
	BLURRING_OFFSET(3));
#elif SAMPLES == 8
const vec2 kernel[9] = vec2[](
	BLURRING_OFFSET(-4),
	BLURRING_OFFSET(-3),
	BLURRING_OFFSET(-2),
	BLURRING_OFFSET(-1),
	BLURRING_OFFSET(0),
	BLURRING_OFFSET(1),
	BLURRING_OFFSET(2),
	BLURRING_OFFSET(3),
	BLURRING_OFFSET(4));
#endif

layout(location = 0) out COL_TYPE fFragColor;

void main()
{
	// the color
	COL_TYPE col = texture(img, vTexCoords + kernel[0]);

	// Get the samples
	for(int i = 1; i < SAMPLES; i++)
	{
		col += texture(img, vTexCoords + kernel[i]);
	}

	fFragColor = col * (1.0 / float(SAMPLES));
}
