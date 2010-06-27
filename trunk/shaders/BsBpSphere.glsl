#pragma anki vertShaderBegins

attribute vec3 position;
attribute vec3 normal;
//attribute vec2 texCoords;

uniform mat4 modelViewProjectionMat;
uniform mat3 normalMat;

//varying vec2 texCoords_v2f;
varying vec3 normalV2f;

void main()
{
	//texCoords_v2f = texCoords;
	normalV2f = normalMat * normal;
	gl_Position = modelViewProjectionMat * vec4(position, 1.0);
}

#pragma anki fragShaderBegins

#pragma anki include "shaders/pack.glsl"

uniform sampler2D ppsPrePassFai;
uniform sampler2D noiseMap;
//varying vec2 texCoords_v2f;
varying vec3 normalV2f;
uniform vec2 rendererSize;  

void main()
{
	/*vec3 _noise = DecodeNormal(texture2D(noiseMap, gl_FragCoord.xy*vec2(1.0/R_W, 1.0/R_H)).rg).rgb;
	_noise = _noise * 2 - 1;
	_noise *= 7.0;*/

	//vec4 _texel = texture2D(isFai, gl_FragCoord.xy*vec2(1.0/R_W, 1.0/R_H));

	vec3 z = vec3(0.0, 0.0, 1.0);

	float factor = normalV2f.z*3.0;
	factor = pow(factor, 6);
	vec3 col = texture2D(ppsPrePassFai, (gl_FragCoord.xy+(factor*20))*vec2(1.0/rendererSize.x, 1.0/rendererSize.y)).rgb * 0.75;
	col.r *= factor;
	col.b *= cos(factor*3.14);
	gl_FragData[0] = vec4(col, factor);
	//gl_FragData[0] = vec4(vec3(1), factor);
}
