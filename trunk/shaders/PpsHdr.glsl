/**
 * Pass 0 horizontal blur, pass 1 vertical, pass 2 median
 * The offset of the blurring depends on the luminance of the current fragment
 */

#pragma anki vertShaderBegins

#pragma anki include "shaders/SimpleVert.glsl"

#pragma anki fragShaderBegins

uniform sampler2D fai; ///< Its the IS FAI
uniform float exposure;

varying vec2 vTexCoords;


void main()
{
	vec3 _color_ = texture2D(fai, vTexCoords).rgb;
	float _luminance_ = dot(vec3(0.30, 0.59, 0.11), _color_); // AKA luminance
	const float _brightMax_ = 4.0;
	float _yd_ = exposure * (exposure/_brightMax_ + 1.0) / (exposure + 1.0) * _luminance_;
	_color_ *= _yd_;
	gl_FragData[0].rgb = _color_;
}
