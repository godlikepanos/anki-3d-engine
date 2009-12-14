// convert to linear depth

float LinearizeDepth( in float depth_, in float znear_, in float zfar_ )
{
	return (2.0 * znear_) / (zfar_ + znear_ - depth_ * (zfar_ - znear_));
}

float ReadFromTexAndLinearizeDepth( in sampler2D depth_map_, in vec2 tex_coord_, in float znear_, in float zfar_ )
{
	return LinearizeDepth( texture2D( depth_map_, tex_coord_ ).r, znear_, zfar_ );
}
