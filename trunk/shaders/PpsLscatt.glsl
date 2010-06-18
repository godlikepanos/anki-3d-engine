#pragma anki vertShaderBegins

#pragma anki include "shaders/SimpleVert.glsl"

#pragma anki fragShaderBegins

const int SAMPLES_NUM = 100;

float exposure = 2.0;
float decay = 0.8;
float density = 1.0;
float weight = 0.3;
uniform vec2 lightPosScreenSpace = vec2(0.5, 0.5);

uniform sampler2D msDepthFai;
uniform sampler2D isFai;

varying vec2 texCoords;



void main()
{
	vec2 delta_tex_coord = texCoords - lightPosScreenSpace;
	vec2 texCoords2 = texCoords;
	delta_tex_coord *= 1.0 / float(SAMPLES_NUM) * density;
	float illumination_decay = 1.0;

	gl_FragData[0].rgb = vec3(0.0);

	for( int i=0; i<SAMPLES_NUM; i++ )
	{
		texCoords2 -= delta_tex_coord;

		float depth = texture2D( msDepthFai, texCoords2 ).r;
		if( depth != 1.0 ) // if the fragment is not part of the skybox dont bother continuing
		{
			illumination_decay *= decay;
			continue;
		}

		vec3 sample = texture2D( isFai, texCoords2 ).rgb;			
		sample *= illumination_decay * weight;
		gl_FragData[0].rgb += sample;
		illumination_decay *= decay;
	}

	gl_FragData[0].rgb *= exposure;
	
	//gl_FragData[0].rgb = texture2D( isFai, texCoords ).rgb;
	//gl_FragData[0].rgb = vec3(1, 1, 0);
}
