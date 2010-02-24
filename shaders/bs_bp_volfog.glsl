#pragma anki vertShaderBegins

void main()
{
	gl_Position = ftransform();
}

#pragma anki fragShaderBegins

#pragma anki include "shaders/linear_depth.glsl"

uniform sampler2D ms_depth_fai;

void main()
{
	vec2 tex_size_ = textureSize(ms_depth_fai, 0);
	vec2 tex_coords_ = gl_FragCoord.xy/tex_size_;
	
	float depth_exp_ = texture2D( ms_depth_fai, tex_coords_ ).r;
	if( depth_exp_ == 1 ) discard;
	
	float depth_ = LinearizeDepth( depth_exp_, 0.1, 10.0 );
	gl_FragColor = vec4( depth_ );
	gl_FragColor = vec4( 0.5 );
}
