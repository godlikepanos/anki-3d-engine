/**
 * @file
 * Generic shader program for Gausian blur
 * Switches: VPASS or HPASS, COL_RGBA or COL_RGB or COL_R
 */

#pragma anki vertShaderBegins

#pragma anki attribute position 0
attribute vec2 position;

void main()
{
	gl_Position = vec4(position * 2.0 - 1.0, 0.0, 1.0);
}

#pragma anki fragShaderBegins


/*
 * Preprocessor switcher sanity checks
 */
#if !defined(VPASS) && !defined(HPASS)
	#error "See file"
#endif

#if !(defined(COL_RGBA) || defined(COL_RGB) || defined(COL_R))
	#error "See file"
#endif


uniform sampler2D img; ///< Input FAI
uniform vec2 imgSize = vec2(100.0, 100.0);

varying vec2 texCoords;


/*
 * Determin color type
 */
#if defined(COL_RGBA)
	#define COL_TYPE vec4
#elif defined(COL_RGB)
	#define COL_TYPE vec3
#elif defined(COL_R)
	#define COL_TYPE float
#endif

/*
 * Determin tex fetch
 */
#if defined(COL_RGBA)
	#define TEX_FETCH rgba
#elif defined(COL_RGB)
	#define TEX_FETCH rgb
#elif defined(COL_R)
	#define TEX_FETCH r
#endif


void main()
{
	const float _offset_[3] = float[](0.0, 1.3846153846, 3.2307692308);
	const float _weight_[3] = float[](0.2255859375, 0.314208984375, 0.06982421875);

	COL_TYPE _col_ = texture2D(img, gl_FragCoord.xy / imgSize).TEX_FETCH * weight[0];

	for(int i=1; i<3; i++)
	{
		#if defined(HPASS)
			_col_ += texture2D(img, (gl_FragCoord.xy + vec2(0.0, offset[i])) / imgSize.x).TEX_FETCH * weight[i];
			_col_ += texture2D(img, (gl_FragCoord.xy - vec2(0.0, offset[i])) / imgSize.x).TEX_FETCH * weight[i];
		#elif defined(VPASS)
			_col_ += texture2D(img, (gl_FragCoord.xy + vec2(offset[i], 0.0)) / imgSize.y).TEX_FETCH * weight[i];
			_col_ += texture2D(img, (gl_FragCoord.xy - vec2(offset[i], 0.0)) / imgSize.y).TEX_FETCH * weight[i];
		#endif
	}

	gl_FragColor[0].TEX_FETCH = _col_;
}
