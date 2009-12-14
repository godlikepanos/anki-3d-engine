uniform sampler2D colormap;
uniform sampler2D noisemap;
uniform float timer;
varying vec2 txtr_coords;

#define PI 3.14159265358979323846


void main()
{
	vec3 _noise_vec;
	vec2 displacement;
	float scaled_timer;

	displacement = txtr_coords;

	scaled_timer = timer*0.4;

	displacement.x -= scaled_timer;
	displacement.y -= scaled_timer;

	_noise_vec = normalize( texture2D(noisemap, displacement).xyz );
	_noise_vec = (_noise_vec * 2.0 - 1.0);

	// edge_factor is a value that varies from 1.0 to 0.0 and then again to 1.0. Its zero in the edge of the skybox quad
	// and white in the middle of the quad. We use it in order to reduse the distortion at the edges because of artefacts
	float _edge_factor = sin( txtr_coords.s * PI ) * sin( txtr_coords.t * PI );

	const float _strength_factor = 0.015;

	_noise_vec = _noise_vec * _strength_factor * _edge_factor;

	gl_FragData[1] = texture2D(colormap, txtr_coords + _noise_vec.xy); // write to the diffuse buffer
}

