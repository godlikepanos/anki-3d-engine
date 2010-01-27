#pragma anki vert_shader_begins

#pragma anki include "shaders/simple_vert.glsl"

#pragma anki frag_shader_begins

#pragma anki include "shaders/photoshop_filters.glsl"
#pragma anki include "shaders/median_filter.glsl"

uniform sampler2D is_fai;
uniform sampler2D pps_ssao_fai;
uniform sampler2D ms_normal_fai;
uniform sampler2D pps_hdr_fai;
uniform sampler2D pps_lscatt_fai;

varying vec2 tex_coords;


/*
=======================================================================================================================================
GrayScale                                                                                                                             =
=======================================================================================================================================
*/
vec3 GrayScale( in vec3 _col )
{
	float _grey = (_col.r + _col.g + _col.b) * 0.333333333; // aka: / 3.0
	return vec3(_grey);
}


/*
=======================================================================================================================================
Saturation                                                                                                                            =
=======================================================================================================================================
*/
vec3 Saturation( in vec3 _col, in float _satur_factor )
{
	const vec3 lumCoeff = vec3 ( 0.2125, 0.7154, 0.0721);

	vec3 intensity = vec3( dot(_col, lumCoeff) );
	return mix( intensity, _col, _satur_factor );
}


/*
=======================================================================================================================================
EdgeAA                                                                                                                                =
=======================================================================================================================================
*/
#if defined(_EDGEAA_)

//float LinearizeDepth( in float _depth )
//{
//	return (2.0 * camerarange.x) / (camerarange.y + camerarange.x - _depth * (camerarange.y - camerarange.x));
//}
//
//float ReadLinearDepth( in vec2 coord )
//{
//	return LinearizeDepth( texture2D( depth_map, coord ).r );
//}

vec3 EdgeAA()
{
	const vec2 pixelsize = vec2( 1.0/(R_W*R_Q), 1.0/(R_H*R_Q) );
	const vec2 kernel[8] = vec2[]( vec2(-1.0,1.0), vec2(1.0,-1.0), vec2(-1.0,-1.0), vec2(1.0,1.0), vec2(-1.0,0.0), vec2(1.0,0.0), vec2(0.0,-1.0), vec2(0.0,1.0) );
	const float weight = 1.0;

	vec3 tex = texture2D( ms_normal_fai, tex_coords ).rgb;
	float factor = -0.5;

	for( int i=0; i<4; i++ )
	{
		vec3 t = texture2D( ms_normal_fai, tex_coords + kernel[i]*pixelsize ).rgb;
		t -= tex;
		factor += dot(t, t);
	}

	factor = min(1.0, factor) * weight;

	//return vec3(factor);
	//if( factor < 0.01 ) return texture2D( is_fai, tex_coords ).rgb;

	vec3 color = vec3(0.0);

	for( int i=0; i<8; i++ )
	{
		color += texture2D( is_fai, tex_coords + kernel[i]*pixelsize*factor ).rgb;
	}

	color += 2.0 * texture2D( is_fai, tex_coords ).rgb;

	return color*(1.0/9.0);

//	const float aob = 1.0; // area of blur
//
//	vec2 kernel[8];
//	kernel[0] = vec2(-aob,aob);
//	kernel[1] = vec2(aob,-aob);
//	kernel[2] = vec2(-aob,-aob);
//	kernel[3] = vec2(aob,aob);
//	kernel[4] = vec2(-aob,0);
//	kernel[5] = vec2(aob,0);
//	kernel[6] = vec2(0,-aob);
//	kernel[7] = vec2(0,aob);
//
//	vec2 pixelsize = vec2( 1.0/R_W, 1.0/R_H );
//
//	float d = ReadLinearDepth( tex_coords );
//	float factor = 0.0;
//
//	const float weight = 8.0;
//
//	for( int i=0; i<4; i++ )
//	{
//		float ds = ReadLinearDepth( tex_coords + kernel[i]*pixelsize );
//
//		factor += abs(d - ds);
//	}
//
//	factor = (factor / 4)*weight;
//
//	//return vec3(factor);
//
//	/*if( factor < 0.001 )
//	{
//		return texture2D( is_fai, tex_coords ).rgb;
//	}*/
//
//	vec3 color = vec3(0.0);
//
//	for( int i=0; i<8; i++ )
//	{
//		color += texture2D( is_fai, tex_coords + kernel[i]*pixelsize*factor ).rgb;
//	}
//
//	color += 2.0 * texture2D( is_fai, tex_coords ).rgb;
//
//	return color*(1.0/10.0);
}
#endif


/*
=======================================================================================================================================
main                                                                                                                                  =
=======================================================================================================================================
*/
void main (void)
{
	vec3 color;

	#if defined(_EDGEAA_)
		color = EdgeAA();
	#else
		color = texture2D( is_fai, tex_coords ).rgb;
	#endif

	/*const float gamma = 0.7;
	color.r = pow(color.r, 1.0 / gamma);
	color.g = pow(color.g, 1.0 / gamma);
	color.b = pow(color.b, 1.0 / gamma);*/

	#if defined(_SSAO_)
		float ssao_factor = texture2D( pps_ssao_fai, tex_coords ).a;
		color *= ssao_factor;
	#endif

	#if defined(_LSCATT_)
		vec3 lscatt = texture2D( pps_lscatt_fai, tex_coords ).rgb;
		color += lscatt;
	#endif

	#if defined(_HDR_)
		vec3 hdr = texture2D( pps_hdr_fai, tex_coords ).rgb;
		color += hdr;
	#endif

	color = BlendHardLight( vec3(0.6, 0.62, 0.4), color );

	gl_FragData[0].rgb = color;

	//gl_FragColor = vec4( color, 1.0 );
	//gl_FragColor = vec4( GrayScale(gl_FragColor.rgb), 1.0 );
	//gl_FragData[0] = vec4( ssao_factor );
	//gl_FragData[0] = vec4( lscatt, 1.0 );
	//gl_FragData[0] = vec4( bloom_factor );
	//gl_FragColor = vec4( EdgeAA(), 1.0 );
	//gl_FragColor = texture2D( pps_boom_fai, tex_coords );
	//gl_FragColor = texture2D( is_fai, tex_coords );
	//gl_FragData[0].rgb = UnpackNormal( texture2D( ms_normal_fai, tex_coords ).rg );
	//gl_FragData[0] = vec4( hdr, 1.0 );
}

