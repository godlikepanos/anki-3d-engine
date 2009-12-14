uniform sampler2D diffuse_map, noise_map;
varying vec2 tex_coords;
varying vec3 normal;

vec3 DecodeNormal( in vec2 _enc )
{
	float _scale = 1.7777;
	vec3 _nn = vec3( _enc*vec2(2.0*_scale, 2.0*_scale) + vec2(-_scale, -_scale), 1.0 );
	float _g = 2.0 / dot(_nn.xyz, _nn.xyz);
	vec3 _normal;
	_normal.xy = _g * _nn.xy;
	_normal.z = _g - 1.0;
	return _normal;
}

void main()
{
	/*vec3 _noise = DecodeNormal( texture2D( noise_map, gl_FragCoord.xy*vec2( 1.0/R_W, 1.0/R_H ) ).rg ).rgb;
	_noise = _noise * 2 - 1;
	_noise *= 7.0;*/
	vec4 _texel = texture2D( diffuse_map, (gl_FragCoord.xy+(normal.xy*2))*vec2( 1.0/R_W, 1.0/R_H ) );


	//vec4 _texel = texture2D( diffuse_map, tex_coords );

	gl_FragColor = _texel;
}
