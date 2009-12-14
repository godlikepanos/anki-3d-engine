//
uniform sampler2D ms_depth_fai;

void main()
{
	vec2 tex_coords_ = gl_FragCoord.xy*vec2( 1.0/R_W, 1.0/R_H );
	
	float depth_exp_ = texture2D( ms_depth_fai, tex_coords_ ).r;
	if( depth_exp_ == 1 ) discard;
	
	float depth_ = LinearizeDepth( depth_exp_, 0.1, 10.0 );
	gl_FragColor = vec4( depth_ );
}
