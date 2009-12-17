#pragma anki vert_shader_begins

varying vec2 tex_coords;
varying vec3 normal;

void main()
{
	tex_coords = gl_MultiTexCoord0.xy;
	normal = gl_NormalMatrix * gl_Normal;
	gl_Position = ftransform();
}

#pragma anki frag_shader_begins

#pragma anki include "shaders/pack.glsl"

uniform sampler2D diffuse_map, noise_map;
varying vec2 tex_coords;
varying vec3 normal;

void main()
{
	/*vec3 _noise = DecodeNormal( texture2D( noise_map, gl_FragCoord.xy*vec2( 1.0/R_W, 1.0/R_H ) ).rg ).rgb;
	_noise = _noise * 2 - 1;
	_noise *= 7.0;*/
	vec4 _texel = texture2D( diffuse_map, (gl_FragCoord.xy+(normal.xy*2))*vec2( 1.0/R_W, 1.0/R_H ) );


	//vec4 _texel = texture2D( diffuse_map, tex_coords );

	gl_FragColor = _texel;
}
