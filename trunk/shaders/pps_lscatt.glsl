#pragma anki vertShaderBegins

#pragma anki include "shaders/simple_vert.glsl"

#pragma anki fragShaderBegins

const int SAMPLES_NUM = 100;

float exposure = 2.0;
float decay = 0.8;
float density = 1.0;
float weight = 0.3;
uniform vec2 light_pos_screen_space = vec2(0.5, 0.5);

#pragma anki uniform ms_depth_fai 0
uniform sampler2D ms_depth_fai;
#pragma anki uniform is_fai 1
uniform sampler2D is_fai;

varying vec2 tex_coords;



void main()
{
	vec2 delta_tex_coord = tex_coords - light_pos_screen_space;
	vec2 tex_coords2 = tex_coords;
	delta_tex_coord *= 1.0 / float(SAMPLES_NUM) * density;
	float illumination_decay = 1.0;

	gl_FragData[0].rgb = vec3(0.0);

	for( int i=0; i<SAMPLES_NUM; i++ )
	{
		tex_coords2 -= delta_tex_coord;

		float depth = texture2D( ms_depth_fai, tex_coords2 ).r;
		if( depth != 1.0 ) // if the fragment is not part of the skybox dont bother continuing
		{
			illumination_decay *= decay;
			continue;
		}

		vec3 sample = texture2D( is_fai, tex_coords2 ).rgb;			
		sample *= illumination_decay * weight;
		gl_FragData[0].rgb += sample;
		illumination_decay *= decay;
	}

	gl_FragData[0].rgb *= exposure;
	
	//gl_FragData[0].rgb = texture2D( is_fai, tex_coords ).rgb;
	//gl_FragData[0].rgb = vec3(1, 1, 0);
}
