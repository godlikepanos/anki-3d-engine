/**
 * Pass 0 horizontal blur, pass 1 vertical, pass 2 median
 * The offset of the bluring depends on the luminance of the current fragment
 */

#pragma anki vertShaderBegins

#pragma anki include "shaders/simple_vert.glsl"

#pragma anki fragShaderBegins

#pragma anki include "shaders/median_filter.glsl"

varying vec2 texCoords;

uniform sampler2D fai; ///< Its the IS FAI

void main()
{
	//gl_FragData[0].rgb = texture2D( fai, texCoords).rgb;return;

	/*if( texCoords.x*R_W > textureSize(fai,0).x/2 )
		gl_FragData[0].rgb = vec3( 1, 0, 0 );
	else
		gl_FragData[0].rgb = vec3( 0, 1, 0 );
	return;*/

	#if defined( _PPS_HDR_PASS_0_ ) || defined( _PPS_HDR_PASS_1_ )
		/*vec3 color = texture2D( fai, texCoords ).rgb
		float luminance = dot( vec3(0.30, 0.59, 0.11) * color ); 
	
		const float maxAdditionalOffset = 2.0;
		float additionalOffset = luminance * maxAdditionalOffset;

		#if defined( _PPS_HDR_PASS_0_ )
			float offset = 1.0 / IS_FAI_WIDTH * additionalOffset;
		#else
			float offset = 1.0 / PASS0_HEIGHT * additionalOffset;
		#endif

		const int KERNEL_SIZE = 8;
		float kernel[KERNEL_SIZE] = float[]( -3.0 * offset, -2.0 * offset, -1.0 * offset, 1.0 * offset, 2.0 * offset,
				                                  3.0 * offset, -4.0 * offset, 4.0 * offset );

		for( int i=0; i<KERNEL_SIZE; i++ )
		{
			#if defined( _PPS_HDR_PASS_0_ )
				color += texture2D( fai, texCoords + vec2(kernel[i], 0.0) ).rgb;
			#else // _PPS_HDR_PASS_1_
				color += texture2D( fai, texCoords + vec2(0.0, kernel[i]) ).rgb;
			#endif
		}

		const float denominator = 1.0 / (KERNEL_SIZE + 1); // Opt to make sure its const
		gl_FragData[0].rgb = color * denominator;*/
		
		vec3 color = texture2D( fai, texCoords ).rgb
		float luminance = dot( vec3(0.30, 0.59, 0.11) * color ); 
	
		#if defined( _PPS_HDR_PASS_0_ )
			const float OFFSET = 1.0 / IS_FAI_WIDTH;
		#else
			const float OFFSET = 1.0 / PASS0_HEIGHT;
		#endif

		const float MAX_SAMPLES_NUM = 12.0;
		float samplesNum = MAX_SAMPLES_NUM * luminance;
		float samplesNumDiv2 = samplesNum/2.0;

		for( float s=-samplesNumDiv2; s<=samplesNumDiv2; s += 1.0 )
		{
			#if defined( _PPS_HDR_PASS_0_ )
				color += texture2D( fai, texCoords + vec2( s * OFFSET, 0.0 ) ).rgb;
			#else // _PPS_HDR_PASS_1_
				color += texture2D( fai, texCoords + vec2( 0.0, OFFSET * offset ) ).rgb;
			#endif
		}
		
		gl_FragData[0].rgb = color / ( floor(samplesNum) + 1.0 );

	#else // _PPS_HDR_PASS_2_
		//vec3 color = MedianFilterRGB( fai, texCoords );
		vec3 color = texture2D( fai, texCoords ).rgb;

		float Y = dot(vec3(0.30, 0.59, 0.11), color); // AKA luminance
		const float exposure = 4.0;
		const float brightMax = 4.0;
		float YD = exposure * (exposure/brightMax + 1.0) / (exposure + 1.0) * Y;
		color *= YD;
		gl_FragData[0].rgb = color;
	#endif
}
