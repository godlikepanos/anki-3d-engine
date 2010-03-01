#pragma anki vertShaderBegins

#pragma anki include "shaders/simple_vert.glsl"

#pragma anki fragShaderBegins

#pragma anki include "shaders/median_filter.glsl"

varying vec2 texCoords;

#pragma anki uniform tex 0
uniform sampler2D tex;

void main()
{
	//gl_FragData[0].rgb = texture2D( tex, texCoords).rgb;return;

	/*if( texCoords.x*R_W > textureSize(tex,0).x/2 )
		gl_FragData[0].rgb = vec3( 1, 0, 0 );
	else
		gl_FragData[0].rgb = vec3( 0, 1, 0 );
	return;*/

	#if defined( _PPS_HDR_PASS_0_ ) || defined( _PPS_HDR_PASS_1_ )
		const float super_offset = 2.5;

		#if defined( _PPS_HDR_PASS0_ )
			float offset = 1.0 / float(textureSize(tex,0).x) * super_offset;
		#else
			float offset = 1.0 / float(textureSize(tex,0).y) * super_offset;
		#endif

		const int KERNEL_SIZE = 9;
		float kernel[KERNEL_SIZE] = float[]( -3.0 * offset, -2.0 * offset, -1.0 * offset, 0.0 * offset, 1.0 * offset, 2.0 * offset,
				                                  3.0 * offset, -4.0 * offset, 4.0 * offset );

		vec3 color = vec3(0.0);

		for( int i=0; i<KERNEL_SIZE; i++ )
		{
			#if defined( _PPS_HDR_PASS_0_ )
				color += texture2D( tex, texCoords + vec2(kernel[i], 0.0) ).rgb;
			#else // _PPS_HDR_PASS_1_
				color += texture2D( tex, texCoords + vec2(0.0, kernel[i]) ).rgb;
			#endif
		}

		gl_FragData[0].rgb = color / KERNEL_SIZE;

	#else // _PPS_HDR_PASS_2_
		vec3 color = MedianFilterRGB( tex, texCoords );
		//vec3 color = texture2D( tex, texCoords ).rgb;

		float Y = dot(vec3(0.30, 0.59, 0.11), color);
		const float exposure = 4.0;
		const float brightMax = 4.0;
		float YD = exposure * (exposure/brightMax + 1.0) / (exposure + 1.0) * Y;
		color *= YD;
		gl_FragData[0].rgb = color;
	#endif
}
