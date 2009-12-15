#pragma anki vert_shader_begins

#pragma anki include "shaders/simple_vert.glsl"

#pragma anki frag_shader_begins

// the first pass is for horizontal blur and the second for vertical

varying vec2 tex_coords;

#if defined( _BLOOM_VBLUR_ )
	uniform sampler2D is_fai;
#elif defined ( _BLOOM_FINAL_ )
	uniform sampler2D bloom_final_fai; // ToDo: rename
#else
	#error "something is wrong"
#endif



/*
=======================================================================================================================================
main                                                                                                                                  =
=======================================================================================================================================
*/
void main (void)
{
	const float super_offset = 1.;
	#if defined( _BLOOM_VBLUR_ )
		const float offset = 1.0 / (R_W*R_Q*BLOOM_RENDERING_QUALITY) * super_offset; // 1 / width_of_the_IS_FAI     !!!!!!!!!!!!!!!!!!!!!!!
	#else
		const float offset = 1.0 / (R_H*R_Q*BLOOM_RENDERING_QUALITY) * super_offset; // 1 / height_of_the_BLOOM_FAI
	#endif

	const int kernel_size = 7;
	float kernel[kernel_size];
	kernel[0] = -3.0 * offset;
	kernel[1] = -2.0 * offset;
	kernel[2] = -1.0 * offset;
	kernel[3] = 0.0 * offset;
	kernel[4] = 1.0 * offset;
	kernel[5] = 2.0 * offset;
	kernel[6] = 3.0 * offset;
	/*kernel[7] = -4.0 * offset;
	kernel[8] = 4.0 * offset;
	kernel[9] = -5.0 * offset;
	kernel[10] = 5.0 * offset;*/

	//float bloom_factor = 0.0; // ORIGINAL BLOOM CODE
	vec3 bloom = vec3(0.0);

	for( int i=0; i<kernel_size; i++ )
	{
		#if defined( _BLOOM_VBLUR_ )
			vec3 color = texture2D( is_fai, tex_coords + vec2(kernel[i], 0.0) ).rgb;

			float Y = dot(vec3(0.30, 0.59, 0.11), color);
			const float exposure = 4.0;
			const float brightMax = 4.0;
			float YD = exposure * (exposure/brightMax + 1.0) / (exposure + 1.0) * Y;
			color *= YD;

			bloom += color;
			//vec3 color = texture2D( is_fai, tex_coords + vec2(kernel[i], 0.0) ).rgb; // ORIGINAL BLOOM CODE
			//bloom_factor += dot(tex,tex); // ORIGINAL BLOOM CODE
		#else
			vec3 color = texture2D( bloom_final_fai, tex_coords + vec2(0.0, kernel[i]) ).rgb;
			bloom += color;

			//float factor = texture2D( bloom_final_fai, tex_coords + vec2(0.0, kernel[i]) ).r; // ORIGINAL BLOOM CODE
			//bloom_factor += factor; // ORIGINAL BLOOM CODE
		#endif
	}

	gl_FragColor.rgb = bloom * (1.0/kernel_size);
	//gl_FragColor.r = bloom_factor * (1.0/7.0); // ORIGINAL BLOOM CODE
}

