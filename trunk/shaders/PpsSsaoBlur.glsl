#pragma anki vertShaderBegins

#pragma anki include "shaders/SimpleVert.glsl"

#pragma anki fragShaderBegins

#pragma anki include "shaders/median_filter.glsl"

uniform sampler2D tex;

in vec2 vTexCoords;

layout(location = 0) out float fFragColor;

void main()
{
	#if defined( _PPS_SSAO_PASS_0_ )
		float offset = 1.0 / PASS0_FAI_WIDTH;
	#else
		float offset = 1.0 / PASS1_FAI_HEIGHT;
	#endif
	const int KERNEL_SIZE = 7;
	float kernel[KERNEL_SIZE] = float[]( 0.0 * offset, 
	                                    -1.0 * offset, 1.0 * offset,
	                                    -2.0 * offset, 2.0 * offset,
																			-3.0 * offset, 3.0 * offset/*,
																			-4.0 * offset, 4.0 * offset,
																			-5.0 * offset, 5.0 * offset,
																			-6.0 * offset, 6.0 * offset*/ );

	float factor = 0.0;
	for(int i=0; i<KERNEL_SIZE; i++)
	{
		#if defined( _PPS_SSAO_PASS_0_ )
			factor += texture2D( tex, vTexCoords + vec2(kernel[i], 0.0) ).a;
		#else
			factor += texture2D( tex, vTexCoords + vec2(0.0, kernel[i]) ).a;
		#endif		
	}
	fFragColor = factor / float(KERNEL_SIZE);
}
