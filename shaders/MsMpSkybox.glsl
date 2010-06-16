#pragma anki vertShaderBegins

attribute vec3 position;
attribute vec2 texCoords;

uniform mat4 viewProjectionMat;
uniform mat4 viewMat;

varying vec2 texCoordsV2f;

void main()
{
	texCoordsV2f = texCoords;
	
	....
	gl_Position = ;
}

#pragma anki fragShaderBegins

uniform sampler2D colorMap;
uniform sampler2D noiseMap;
uniform float timer;
uniform vec3 sceneAmbientCol;
varying vec2 texCoordsV2f;

#define PI 3.14159265358979323846


void main()
{
	vec3 _noise_vec;
	vec2 displacement;
	float scaled_timer;

	displacement = texCoordsV2f;

	scaled_timer = timer*0.4;

	displacement.x -= scaled_timer;
	displacement.y -= scaled_timer;

	_noise_vec = normalize( texture2D(noisemap, displacement).xyz );
	_noise_vec = (_noise_vec * 2.0 - 1.0);

	// edge_factor is a value that varies from 1.0 to 0.0 and then again to 1.0. Its zero in the edge of the skybox quad
	// and white in the middle of the quad. We use it in order to reduse the distortion at the edges because of artefacts
	float _edge_factor = sin( texCoordsV2f.s * PI ) * sin( texCoordsV2f.t * PI );

	const float _strength_factor = 0.015;

	_noise_vec = _noise_vec * _strength_factor * _edge_factor;

	gl_FragData[1].rgb = texture2D(colormap, texCoordsV2f + _noise_vec.xy).rgb * sceneAmbientCol; // write to the diffuse buffer
}

