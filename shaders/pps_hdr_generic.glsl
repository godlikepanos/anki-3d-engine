#pragma anki vert_shader_begins

#pragma anki include "shaders/simple_vert.glsl"

#pragma anki frag_shader_begins

#pragma anki include "shaders/median_filter.glsl"

varying vec2 tex_coords;

#pragma anki uniform tex 0
uniform sampler2D tex;

void main()
{
	//gl_FragData[0].rgb = texture2D( tex, tex_coords).rgb;return;

	/*if( tex_coords.x*R_W > textureSize(tex,0).x/2 )
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
		float kernel[KERNEL_SIZE];
		kernel[0] = -3.0 * offset;
		kernel[1] = -2.0 * offset;
		kernel[2] = -1.0 * offset;
		kernel[3] = 0.0 * offset;
		kernel[4] = 1.0 * offset;
		kernel[5] = 2.0 * offset;
		kernel[6] = 3.0 * offset;
		kernel[7] = -4.0 * offset;
		kernel[8] = 4.0 * offset;
		/*kernel[9] = -5.0 * offset;
		kernel[10] = 5.0 * offset;*/

		vec3 color = vec3(0.0);

		for( int i=0; i<KERNEL_SIZE; i++ )
		{
			#if defined( _PPS_HDR_PASS_0_ )
				color += texture2D( tex, tex_coords + vec2(kernel[i], 0.0) ).rgb;
			#else // _PPS_HDR_PASS_1_
				color += texture2D( tex, tex_coords + vec2(0.0, kernel[i]) ).rgb;
			#endif
		}

		gl_FragData[0].rgb = color / KERNEL_SIZE;

	#else // _PPS_HDR_PASS_2_
		vec3 color = MedianFilterRGB( tex, tex_coords );
		//vec3 color = texture2D( tex, tex_coords ).rgb;

		float Y = dot(vec3(0.30, 0.59, 0.11), color);
		const float exposure = 4.0;
		const float brightMax = 4.0;
		float YD = exposure * (exposure/brightMax + 1.0) / (exposure + 1.0) * Y;
		color *= YD;
		gl_FragData[0].rgb = color;
	#endif
}
