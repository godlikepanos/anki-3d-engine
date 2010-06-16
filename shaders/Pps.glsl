#pragma anki vertShaderBegins

#pragma anki include "shaders/simple_vert.glsl"

#pragma anki fragShaderBegins

#pragma anki include "shaders/photoshop_filters.glsl"
#pragma anki include "shaders/median_filter.glsl"

uniform sampler2D isFai;
uniform sampler2D ppsSsaoFai;
uniform sampler2D msNormalFai;
uniform sampler2D ppsHdrFai;
uniform sampler2D ppsLscattFai;

varying vec2 texCoords;


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

vec3 EdgeAA()
{
	ivec2 texSize = textureSize(msNormalFai,0);
	vec2 pixelsize = vec2( 1.0/(float(texSize.x)), 1.0/(float(texSize.y)) );
	const vec2 kernel[8] = vec2[]( vec2(-1.0,1.0), vec2(1.0,-1.0), vec2(-1.0,-1.0), vec2(1.0,1.0), vec2(-1.0,0.0), vec2(1.0,0.0), vec2(0.0,-1.0), vec2(0.0,1.0) );
	const float weight = 1.0;

	vec3 tex = texture2D( msNormalFai, texCoords ).rgb;
	float factor = -0.5;

	for( int i=0; i<4; i++ )
	{
		vec3 t = texture2D( msNormalFai, texCoords + kernel[i]*pixelsize ).rgb;
		t -= tex;
		factor += dot(t, t);
	}

	factor = min(1.0, factor) * weight;

	//return vec3(factor);
	//if( factor < 0.01 ) return texture2D( isFai, texCoords ).rgb;

	vec3 color = vec3(0.0);

	for( int i=0; i<8; i++ )
	{
		color += texture2D( isFai, texCoords + kernel[i]*pixelsize*factor ).rgb;
	}

	color += 2.0 * texture2D( isFai, texCoords ).rgb;

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
//	float d = ReadLinearDepth( texCoords );
//	float factor = 0.0;
//
//	const float weight = 8.0;
//
//	for( int i=0; i<4; i++ )
//	{
//		float ds = ReadLinearDepth( texCoords + kernel[i]*pixelsize );
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
//		return texture2D( isFai, texCoords ).rgb;
//	}*/
//
//	vec3 color = vec3(0.0);
//
//	for( int i=0; i<8; i++ )
//	{
//		color += texture2D( isFai, texCoords + kernel[i]*pixelsize*factor ).rgb;
//	}
//
//	color += 2.0 * texture2D( isFai, texCoords ).rgb;
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
		color = texture2D( isFai, texCoords ).rgb;
	#endif

	/*const float gamma = 0.7;
	color.r = pow(color.r, 1.0 / gamma);
	color.g = pow(color.g, 1.0 / gamma);
	color.b = pow(color.b, 1.0 / gamma);*/

	#if defined(_SSAO_)
		float ssao_factor = texture2D( ppsSsaoFai, texCoords ).a;
		color *= ssao_factor;
	#endif

	#if defined(_LSCATT_)
		vec3 lscatt = texture2D( ppsLscattFai, texCoords ).rgb;
		color += lscatt;
	#endif

	#if defined(_HDR_)
		vec3 hdr = texture2D( ppsHdrFai, texCoords ).rgb;
		color += hdr;
	#endif

	/*vec4 sample[9];
	for(int i = 0; i < 9; i++)
	{
		sample[i] = texture2D(sampler0, gl_TexCoord[0].st + tc_offset[i]);
	}
	color = (sample[4] * 9.0) - (sample[0] + sample[1] + sample[2] + sample[3] + sample[5] + sample[6] + sample[7] + sample[8]);*/

	color = BlendHardLight( vec3(0.6, 0.62, 0.4), color );

	gl_FragData[0].rgb = color;

	//gl_FragColor = vec4( color, 1.0 );
	//gl_FragColor = vec4( GrayScale(gl_FragColor.rgb), 1.0 );
	//gl_FragData[0] = vec4( ssao_factor );
	//gl_FragData[0] = vec4( lscatt, 1.0 );
	//gl_FragData[0] = vec4( bloom_factor );
	//gl_FragColor = vec4( EdgeAA(), 1.0 );
	//gl_FragColor = texture2D( pps_boom_fai, texCoords );
	//gl_FragColor = texture2D( isFai, texCoords );
	//gl_FragData[0].rgb = UnpackNormal( texture2D( msNormalFai, texCoords ).rg );
	//gl_FragData[0] = vec4( hdr, 1.0 );
}

