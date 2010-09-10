/**
 * @file
 * Generic shader program for Gausian blur inspired by Daniel Rákos' article
 * (http://rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/)
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
uniform vec2 imgSize = vec2(0.0, 0.0);
uniform float blurringDist = 1.0;

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
 * Determine tex fetch
 */
#if defined(COL_RGBA)
	#define TEX_FETCH rgba
#elif defined(COL_RGB)
	#define TEX_FETCH rgb
#elif defined(COL_R)
	#define TEX_FETCH r
#endif


const float _offset_[3] = float[](0.0, 1.3846153846, 3.2307692308);
const float _weight_[3] = float[](0.2255859375, 0.314208984375, 0.06982421875);


void main()
{
	COL_TYPE _col_ = texture2D(img, gl_FragCoord.xy / imgSize).TEX_FETCH * _weight_[0];

	for(int i=1; i<3; i++)
	{
		#if defined(HPASS)
			vec2 _vecOffs_ = vec2(_offset_[i] + blurringDist, 0.0);
		#elif defined(VPASS)
			vec2 _vecOffs_ = vec2(0.0, _offset_[i] + blurringDist);
		#endif

		_col_ += texture2D(img, (gl_FragCoord.xy + _vecOffs_) / imgSize).TEX_FETCH * _weight_[i];
		_col_ += texture2D(img, (gl_FragCoord.xy - _vecOffs_) / imgSize).TEX_FETCH * _weight_[i];
	}

	gl_FragData[0].TEX_FETCH = _col_;
}
