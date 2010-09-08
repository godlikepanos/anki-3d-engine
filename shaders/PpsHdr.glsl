/**
 * Pass 0 horizontal blur, pass 1 vertical, pass 2 median
 * The offset of the blurring depends on the luminance of the current fragment
 */

#pragma anki vertShaderBegins

#pragma anki include "shaders/SimpleVert.glsl"

#pragma anki fragShaderBegins

uniform sampler2D fai; ///< Its the IS FAI
varying vec2 texCoords;


void main()
{
	//gl_FragData[0].rgb = texture2D(fai, texCoords).rgb;return;

	/*if(texCoords.x*R_W > textureSize(fai,0).x/2)
		gl_FragData[0].rgb = vec3(1, 0, 0);
	else
		gl_FragData[0].rgb = vec3(0, 1, 0);
	return;*/

	#if defined(_PPS_HDR_PASS_0_) || defined(_PPS_HDR_PASS_1_)
		vec3 color = texture2D(fai, texCoords).rgb;
		//float luminance = dot(vec3(0.30, 0.59, 0.11), color);

		const float additionalOffset = 2.0;

		#if defined(_PPS_HDR_PASS_0_)
			const float offset = 1.0 / IS_FAI_WIDTH * additionalOffset;
		#else
			const float offset = 1.0 / PASS0_HEIGHT * additionalOffset;
		#endif

		const int SAMPLES_NUM = 6;
		const float kernel[] = float[](-1.0 * offset, 1.0 * offset, -2.0 * offset, 2.0 * offset,
				                            -3.0 * offset, 3.0 * offset, -4.0 * offset, 4.0 * offset,
				                            -5.0 * offset, 5.0 * offset, -6.0 * offset, 6.0 * offset);


		/*const float _offset_[3] = float[](0.0, 1.3846153846, 3.2307692308);
		const float _weight_[3] = float[](0.2255859375, 0.314208984375, 0.06982421875);*/


		for(int i=0; i<SAMPLES_NUM; i++)
		{
			#if defined(_PPS_HDR_PASS_0_)
				color += texture2D(fai, texCoords + vec2(kernel[i], 0.0)).rgb;
			#else // _PPS_HDR_PASS_1_
				color += texture2D(fai, texCoords + vec2(0.0, kernel[i])).rgb;
			#endif
		}

		const float denominator = 1.0 / (SAMPLES_NUM + 1); // Opt to make sure its const
		gl_FragData[0].rgb = color * denominator;
	#else // _PPS_HDR_PASS_2_
		//vec3 color = MedianFilterRGB(fai, texCoords);
		vec3 color = texture2D(fai, texCoords).rgb;

		float Y = dot(vec3(0.30, 0.59, 0.11), color); // AKA luminance
		const float exposure = 4.0;
		const float brightMax = 4.0;
		float YD = exposure * (exposure/brightMax + 1.0) / (exposure + 1.0) * Y;
		color *= YD;
		gl_FragData[0].rgb = color;
	#endif
}
