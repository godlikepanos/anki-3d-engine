// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SHADERS_FUNCTIONS_GLSL
#define ANKI_SHADERS_FUNCTIONS_GLSL

//==============================================================================
vec3 dither(in vec3 col, in float C)
{
	vec3 vDither = vec3(dot(vec2(171.0, 231.0), gl_FragCoord.xy));
	vDither.rgb = fract(vDither.rgb / vec3(103.0, 71.0, 97.0));

	col = col * (255.0 / C) + vDither.rgb;
	col = floor(col) / 255.0;
	col *= C;

	return col;
}

//==============================================================================
float dither(in float col, in float C)
{
	float vDither = dot(vec2(171.0, 231.0), gl_FragCoord.xy);
	vDither = fract(vDither / 103.0);

	col = col * (255.0 / C) + vDither;
	col = floor(col) / 255.0;
	col *= C;

	return col;
}

#endif
