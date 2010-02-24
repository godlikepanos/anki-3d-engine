#pragma anki vertShaderBegins

attribute vec3 position;
attribute vec3 normal;
//attribute vec2 texCoords;

//varying vec2 texCoords_v2f;
varying vec3 normal_v2f;

void main()
{
	//texCoords_v2f = texCoords;
	normal_v2f = gl_NormalMatrix * normal;
	gl_Position = gl_ModelViewProjectionMatrix * vec4(position, 1.0);
}

#pragma anki fragShaderBegins

#pragma anki include "shaders/pack.glsl"

uniform sampler2D diffuse_map, noise_map;
//varying vec2 texCoords_v2f;
varying vec3 normal_v2f;

void main()
{
	/*vec3 _noise = DecodeNormal( texture2D( noise_map, gl_FragCoord.xy*vec2( 1.0/R_W, 1.0/R_H ) ).rg ).rgb;
	_noise = _noise * 2 - 1;
	_noise *= 7.0;*/

	vec4 _texel = texture2D( diffuse_map, (gl_FragCoord.xy+(normal_v2f.z*50))*vec2( 1.0/R_W, 1.0/R_H ) ) * 0.75;
	//vec4 _texel = texture2D( diffuse_map, gl_FragCoord.xy*vec2( 1.0/R_W, 1.0/R_H ) );

	gl_FragData[0] = _texel;

	//if( normal_v2f.z > 0.5 ) discard;

	//gl_FragData[0] = vec4( normal_v2f.z );
}
