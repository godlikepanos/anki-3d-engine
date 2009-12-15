#pragma anki vert_shader_begins

varying vec2 txtr_coords;

void main()
{
	txtr_coords = gl_MultiTexCoord0.xy;
	gl_Position = ftransform();
}

#pragma anki frag_shader_begins

#pragma anki uniform colormap 0
uniform sampler2D colormap;
#pragma anki uniform noisemap 1
uniform sampler2D noisemap;
#pragma anki uniform timer 2
uniform float timer;
#pragma anki uniform scene_ambient_color 3
uniform vec3 scene_ambient_color;
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

	gl_FragData[1].rgb = texture2D(colormap, txtr_coords + _noise_vec.xy).rgb * scene_ambient_color; // write to the diffuse buffer
}

