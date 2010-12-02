#pragma anki vertShaderBegins

#pragma anki include "shaders/SimpleVert.glsl"

#pragma anki fragShaderBegins

#pragma anki include "shaders/photoshop_filters.glsl"

uniform sampler2D ppsPrePassFai;
uniform sampler2D ppsHdrFai;

in vec2 vTexCoords;

layout(location = 0) out vec3 fFragColor;


//======================================================================================================================
// GrayScale                                                                                                           =
//======================================================================================================================
vec3 grayScale(in vec3 col)
{
	float grey = (col.r + col.g + col.b) * 0.333333333; // aka: / 3.0
	return vec3(grey);
}


//======================================================================================================================
// saturation                                                                                                          =
//======================================================================================================================
vec3 saturation(in vec3 col, in float factor)
{
	const vec3 lumCoeff = vec3(0.2125, 0.7154, 0.0721);

	vec3 intensity = vec3(dot(col, lumCoeff));
	return mix(intensity, col, factor);
}


//======================================================================================================================
// main                                                                                                                =
//======================================================================================================================
void main(void)
{
	fFragColor = texture2D(ppsPrePassFai, vTexCoords).rgb;

	/*const float gamma = 0.7;
	color.r = pow(color.r, 1.0 / gamma);
	color.g = pow(color.g, 1.0 / gamma);
	color.b = pow(color.b, 1.0 / gamma);*/

	#if defined(HDR_ENABLED)
		vec3 hdr = texture2D(ppsHdrFai, vTexCoords).rgb;
		fFragColor += hdr;
	#endif

	fFragColor = BlendHardLight(vec3(0.6, 0.62, 0.4), fFragColor);
}

